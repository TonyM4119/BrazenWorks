// SPDX-License-Identifier: MIT
#include "mitcad/io/exchange.hpp"

#include <cstdlib>
#include <algorithm>
#include <cctype>

#include <ShapeFix_Shape.hxx>
#include <Standard_Failure.hxx>
#include <BRepCheck_Analyzer.hxx>
#include <BRepTools.hxx>
#include <BRep_Builder.hxx>
#include <TopoDS_Compound.hxx>

#include "xcaf.hpp"

namespace mitcad::io {

std::optional<Format> format_of(const std::string& path) {
  const std::size_t dot = path.find_last_of('.');
  const std::size_t slash = path.find_last_of("/\\");
  if (dot == std::string::npos || (slash != std::string::npos && dot < slash)) {
    return std::nullopt;
  }
  std::string extension = path.substr(dot + 1);
  std::transform(extension.begin(), extension.end(), extension.begin(),
                 [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
  if (extension == "step" || extension == "stp") {
    return Format::Step;
  }
  if (extension == "iges" || extension == "igs") {
    return Format::Iges;
  }
  if (extension == "stl") {
    return Format::Stl;
  }
  if (extension == "obj") {
    return Format::Obj;
  }
  if (extension == "brep" || extension == "brp") {
    return Format::Brep;
  }
  return std::nullopt;
}

namespace {

Format require_format(const std::string& path) {
  const std::optional<Format> format = format_of(path);
  if (!format) {
    throw Error("unknown file type: " + path);
  }
  return *format;
}

} // namespace

namespace detail {

// PureCAD: Preferences › Design › Heal imported parts that fail the check:
// the app sets PURECAD_HEAL_IMPORTS=1; a body BRepCheck finds invalid goes
// through ShapeFix_Shape, and the fixed shape is kept when it is valid.
std::vector<Body> heal_if_asked(std::vector<Body> bodies) {
  const char* on = std::getenv("PURECAD_HEAL_IMPORTS");
  if (on == nullptr || std::string(on) != "1") {
    return bodies;
  }
  for (Body& body : bodies) {
    try {
      if (body.shape.IsNull() || BRepCheck_Analyzer(body.shape).IsValid()) {
        continue;
      }
      ShapeFix_Shape fix(body.shape);
      fix.Perform();
      const TopoDS_Shape fixed = fix.Shape();
      if (!fixed.IsNull() && BRepCheck_Analyzer(fixed).IsValid()) {
        body.shape = fixed;
      }
    } catch (const Standard_Failure&) {
      // Kept as read.
    }
  }
  return bodies;
}

} // namespace detail

std::vector<Body> read_file(const std::string& path, const Message_ProgressRange& range) {
  switch (require_format(path)) {
  case Format::Step:
    return detail::heal_if_asked(read_step(path, range));
  case Format::Iges:
    return detail::heal_if_asked(read_iges(path, range));
  case Format::Stl:
    return {read_stl(path)};
  case Format::Obj:
    return read_obj(path);
  case Format::Brep: {
    TopoDS_Shape shape;
    if (!BRepTools::Read(shape, path.c_str(), BRep_Builder()) || shape.IsNull()) {
      throw Error("cannot read BREP file " + path);
    }
    return detail::heal_if_asked({Body{detail::file_stem(path), shape, std::nullopt, {}, {}}});
  }
  }
  return {};
}

void write_file(const std::string& path, const std::vector<Body>& bodies,
                const WriteOptions& options) {
  const Format format = require_format(path);
  detail::require_shapes(bodies);
  switch (format) {
  case Format::Step:
    write_step(path, bodies, StepWriteOptions{options.step_schema, options.unit});
    return;
  case Format::Iges:
    write_iges(path, bodies, IgesWriteOptions{options.unit, true});
    return;
  case Format::Stl: {
    std::vector<TopoDS_Shape> shapes;
    for (const Body& body : placed_bodies(bodies)) {
      shapes.push_back(body.shape);
    }
    write_stl(path, shapes, StlWriteOptions{options.stl_format, options.mesh});
    return;
  }
  case Format::Obj:
    write_obj(path, bodies, ObjWriteOptions{options.mesh, options.unit});
    return;
  case Format::Brep:
    write_brep(path, bodies);
    return;
  }
}

void write_brep(const std::string& path, const std::vector<Body>& bodies) {
  detail::require_shapes(bodies);
  const std::vector<Body> placed = placed_bodies(bodies);
  BRep_Builder builder;
  TopoDS_Compound compound;
  builder.MakeCompound(compound);
  for (const Body& body : placed) {
    builder.Add(compound, body.shape);
  }
  const TopoDS_Shape shape = placed.size() == 1 ? placed.front().shape : compound;
  if (!BRepTools::Write(shape, path.c_str())) {
    throw Error("cannot write BREP file " + path);
  }
}

} // namespace mitcad::io

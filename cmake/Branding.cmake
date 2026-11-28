# SPDX-License-Identifier: AGPL-3.0-or-later
# Fork: the product's name and identity come from branding.json at
# the top of the repository. This reads it once and generates brand.hpp
# (namespace brand) in the build tree, on the include path of every target,
# and on Windows brand.rc (the program file's icon and version information).
# A change to branding.json configures again.

set(BRANDING_FILE "${CMAKE_SOURCE_DIR}/branding.json")
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${BRANDING_FILE}")
file(READ "${BRANDING_FILE}" _brand_json)

function(_brand_read key out)
  string(JSON _value ERROR_VARIABLE _error GET "${_brand_json}" "${key}")
  if(_error)
    message(FATAL_ERROR "branding.json: ${key} is missing (${_error})")
  endif()
  set(${out} "${_value}" PARENT_SCOPE)
endfunction()

_brand_read(displayName BRAND_DISPLAY_NAME)
_brand_read(storedName BRAND_STORED_NAME)
_brand_read(slug BRAND_SLUG)
# The product's own version, shown to people (title, About, the program
# file's Properties, the installer). PROJECT_VERSION stays the upstream's:
# designs, installations and the upstream's checks are keyed on it.
_brand_read(version BRAND_VERSION)
if(NOT BRAND_VERSION MATCHES "^([0-9]+)\\.([0-9]+)\\.([0-9]+)$")
  message(FATAL_ERROR "branding.json: version ${BRAND_VERSION} is not <major>.<minor>.<patch>")
endif()
set(BRAND_VERSION_MAJOR "${CMAKE_MATCH_1}")
set(BRAND_VERSION_MINOR "${CMAKE_MATCH_2}")
set(BRAND_VERSION_PATCH "${CMAKE_MATCH_3}")
_brand_read(organizationDomain BRAND_ORGANIZATION_DOMAIN)
_brand_read(repositoryUrl BRAND_REPOSITORY_URL)
_brand_read(upstreamName BRAND_UPSTREAM_NAME)
_brand_read(upstreamUrl BRAND_UPSTREAM_URL)
_brand_read(upstreamUpdateChecks BRAND_UPSTREAM_UPDATE_CHECKS)
_brand_read(upstreamCommunityLinks BRAND_UPSTREAM_COMMUNITY_LINKS)
if(BRAND_UPSTREAM_COMMUNITY_LINKS)
  set(BRAND_UPSTREAM_COMMUNITY_LINKS_CPP true)
else()
  set(BRAND_UPSTREAM_COMMUNITY_LINKS_CPP false)
endif()
_brand_read(iconDir _brand_icon_dir)
# The icon folder: icon.svg, icon-small.svg, png/icon-<size>.png and icon.ico
# (tools/make-brand-icons.sh).
set(BRAND_ICON_DIR "${CMAKE_SOURCE_DIR}/${_brand_icon_dir}")
if(NOT EXISTS "${BRAND_ICON_DIR}/icon.ico")
  message(FATAL_ERROR "branding.json: iconDir ${_brand_icon_dir} has no icon.ico (tools/make-brand-icons.sh)")
endif()
# The program file's name on Windows: <slug>.exe (app/CMakeLists.txt).
set(BRAND_EXE "${BRAND_SLUG}.exe")
if(BRAND_UPSTREAM_UPDATE_CHECKS)
  set(BRAND_UPSTREAM_UPDATE_CHECKS_CPP true)
else()
  set(BRAND_UPSTREAM_UPDATE_CHECKS_CPP false)
endif()

set(BRAND_GENERATED_DIR "${CMAKE_BINARY_DIR}/generated/brand")
configure_file("${CMAKE_SOURCE_DIR}/cmake/brand.hpp.in" "${BRAND_GENERATED_DIR}/brand.hpp" @ONLY)
include_directories(BEFORE "${BRAND_GENERATED_DIR}")
if(WIN32)
  # The resource compiler wants forward slashes or doubled backslashes.
  file(TO_CMAKE_PATH "${BRAND_ICON_DIR}/icon.ico" BRAND_ICON_ICO)
  configure_file("${CMAKE_SOURCE_DIR}/cmake/brand.rc.in" "${BRAND_GENERATED_DIR}/brand.rc" @ONLY)
endif()
message(STATUS "Branding: ${BRAND_DISPLAY_NAME} ${BRAND_VERSION} (${BRAND_SLUG}), based on ${BRAND_UPSTREAM_NAME} ${PROJECT_VERSION}")

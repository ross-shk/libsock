# Expects INC_FILES (";"-separated list of input .inc files) and OUT_FILE.
# Concatenates the inputs into OUT_FILE, dropping any line whose first
# non-blank characters are "%include".
# Works on whole-text regexes so PL/I semicolons can never corrupt the split.
if(NOT DEFINED INC_FILES OR NOT DEFINED OUT_FILE)
  message(FATAL_ERROR "GenDistInc.cmake requires INC_FILES and OUT_FILE")
endif()

set(content "")
foreach(f ${INC_FILES})
  file(READ "${f}" text)
  string(REPLACE "\r\n" "\n" text "${text}")
  # Drop every line whose first non-blank characters are %include.
  # The (^|\n) anchor keeps prose mentions of %include intact.
  string(REGEX REPLACE "(^|\n)[ \t]*%include[^\n]*" "\\1" text "${text}")
  string(APPEND content "${text}")
endforeach()

get_filename_component(outdir "${OUT_FILE}" DIRECTORY)
file(MAKE_DIRECTORY "${outdir}")
file(WRITE "${OUT_FILE}" "${content}")

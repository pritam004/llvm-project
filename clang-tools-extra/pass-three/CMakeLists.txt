set(LLVM_LINK_COMPONENTS support)

add_clang_executable(pass-three
  pass-three.cpp
  )
target_link_libraries(pass-three
  PRIVATE
  clangTooling
  clangBasic
  clangASTMatchers
  )


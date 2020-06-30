set(LLVM_LINK_COMPONENTS support)

add_clang_executable(pass-one
  pass-one.cpp
  )
target_link_libraries(pass-one
  PRIVATE
  clangTooling
  clangBasic
  clangASTMatchers
  )


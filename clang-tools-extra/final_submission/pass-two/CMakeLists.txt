set(LLVM_LINK_COMPONENTS support)

add_clang_executable(pass-two
  pass-two.cpp
  )
target_link_libraries(pass-two
  PRIVATE
  clangTooling
  clangBasic
  clangASTMatchers
  )


set(LLVM_TARGET_DEFINITIONS Mos6502.td)

add_llvm_target(Mos6502CodeGen
  Mos6502TargetMachine.cpp
  )

add_subdirectory(TargetInfo)
add_subdirectory(MCTargetDesc)

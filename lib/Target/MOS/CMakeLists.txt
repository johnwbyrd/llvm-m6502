set(LLVM_TARGET_DEFINITIONS MOS.td)

tablegen(LLVM MOSGenAsmMatcher.inc -gen-asm-matcher)
tablegen(LLVM MOSGenAsmWriter.inc -gen-asm-writer)
tablegen(LLVM MOSGenCallingConv.inc -gen-callingconv)
tablegen(LLVM MOSGenDAGISel.inc -gen-dag-isel)
tablegen(LLVM MOSGenDisassemblerTables.inc -gen-disassembler)
tablegen(LLVM MOSGenInstrInfo.inc -gen-instr-info)
tablegen(LLVM MOSGenMCCodeEmitter.inc -gen-emitter)
tablegen(LLVM MOSGenRegisterInfo.inc -gen-register-info)
tablegen(LLVM MOSGenSubtargetInfo.inc -gen-subtarget)

add_public_tablegen_target(MOSCommonTableGen)

add_llvm_target(MOSCodeGen
  DelaySlotFiller.cpp
  LeonPasses.cpp
  MOSAsmPrinter.cpp
  MOSInstrInfo.cpp
  MOSISelDAGToDAG.cpp
  MOSISelLowering.cpp
  MOSFrameLowering.cpp
  MOSMachineFunctionInfo.cpp
  MOSRegisterInfo.cpp
  MOSSubtarget.cpp
  MOSTargetMachine.cpp
  MOSMCInstLower.cpp
  MOSTargetObjectFile.cpp
  )

add_subdirectory(AsmParser)
add_subdirectory(Disassembler)
add_subdirectory(MCTargetDesc)
add_subdirectory(TargetInfo)

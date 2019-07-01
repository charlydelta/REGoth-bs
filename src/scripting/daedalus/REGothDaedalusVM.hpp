/**\file
 */
#pragma once
#include "DaedalusStack.hpp"
#include <BsPrerequisites.h>
#include <scripting/ScriptVM.hpp>

namespace Daedalus
{
  class DATFile;
  class PARStackOpCode;
}  // namespace Daedalus

namespace REGoth
{
  namespace Scripting
  {
    class DATSymbolStorageLoader;
    class DaedalusClassVarResolver;
    class DaedalusVM : public ScriptVM
    {
    public:
      DaedalusVM(const bs::Vector<bs::UINT8>& datFileData);

    protected:
      /**
       * Executes a script function until it hits its return.
       *
       * Throws, if the function does not exist.
       *
       * @param  name  Name of the script function to execute.
       */
      void executeScriptFunction(const bs::String& name);

      /**
       * Executs a script function until it hits its return.
       *
       * Throws, if the function does not exist.
       *
       * @param  address  Byte-code address of the function to execute.
       */
      void executeScriptFunction(bs::UINT32 address);

      /**
       * Runs the instruction found at the Program Counter and modifies it.
       *
       * @note If this encounteres a CALL-instruction, it will execute the
       *       whole sub-function.
       *
       * @return Whether the script function is not over yet. If this returns
       *         `false` then a Return-statement has been executed.
       */
      bool executeInstructionAtPC();

      /**
       * Runs from the current PC until the function it is in returned.
       */
      void executeUntilReturn();

      /**
       * Looks up the instruction memory at the given address and returns
       * the byte at that location.
       *
       * Throws if the given address is out of range.
       *
       * @param  address  Address to take the byte from.
       *
       * @return Byte at the given address in the instruction memory.
       */
      bs::UINT8 instructionMemoryAt(bs::UINT32 address);

      void fillSymbolStorage() override;

      /**
       * Pops an value from the stack. Also resolves variables.
       */
      bs::INT32 popIntValue();
      float popFloatValue();
      bs::String popStringValue();
      ScriptObjectHandle popInstanceScriptObject();

      /**
       * Pops a reference to an variable stored inside a script symbol.
       *
       * Throws if the value on the stack is not a variable.
       */
      bs::INT32& popIntReference();
      float& popFloatReference();
      bs::String& popStringReference();

      /**
       * Pushes the given variable onto the stack.
       *
       * This function will put the given variable onto the stack with the
       * correct type.
       *
       * Throws if no stack for the symbol type exists.
       *
       * @param  symbolIndex  Index of the symbol to push.
       * @param  arrayIndex   Array-Index of the variable inside the symbol.
       */
      void pushVariable(SymbolIndex symbolIndex, bs::UINT32 arrayIndex);

      /**
       * Register all externals here. Use registerExternal().
       */
      virtual void registerAllExternals(){};

      /**
       * Callback type for a script external function.
       */
      typedef void (DaedalusVM::*externalCallback)(void);

      /**
       * Registers an external function within the VM.
       *
       * @param  name      Name of the external function, UPPERCASE.
       * @param  callback  Callback to be executed when the external function
       *                   is called.
       */
      void registerExternal(const bs::String& name, externalCallback callback);

    protected:
      bs::SPtr<DaedalusClassVarResolver> mClassVarResolver;
      DaedalusStack mStack;

      /**
       * If true, every executed instruction will be logged to the console.
       */
      bool mIsDisassemblerEnabled = false;

    private:
      /**
       * Whether the disassembler should be turned on for the given function.
       */
      bool shouldEnableDisassemblerForFunction(const bs::String& uppercaseName) const;

      /**
       * Whether to hide the given function in the disassembler, it it is active.
       */
      bool shouldHideFunctionInDisassembly(const bs::String& uppercaseName) const;

      /**
       * Disassembles and logs the given opcode in respect ti the call-depth.
       */
      void disassembleAndLogOpcode(const Daedalus::PARStackOpCode& opcode,
                                   const bs::String& lhs = "", const bs::String& rhs = "",
                                   const bs::String& res = "");

      /**
       * Looks up the function at the given address and returns its name
       */
      void findFunctionAtAddressAndLog(bs::UINT32 address);

      /**
       * Program counter register
       */
      bs::UINT32 mPC = 0;

      /**
       * Function nesting counter.
       */
      bs::INT32 mCallDepth = 0;

      bs::SPtr<Daedalus::DATFile> mDatFile;

      // The whole DAT-file, for serialization
      bs::Vector<bs::UINT8> mDatFileData;

      bs::Map<SymbolIndex, externalCallback> mExternals;

    public:
      // Remember, this is abstract, so don't create an rttiCreateEmpty()
      REGOTH_DECLARE_RTTI(DaedalusVM);

    protected:
      DaedalusVM() = default;  // For RTTI
    };
  }  // namespace Scripting
}  // namespace REGoth

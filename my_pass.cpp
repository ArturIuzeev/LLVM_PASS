#include "llvm/Pass.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Function.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"
using namespace llvm;

namespace
{
    struct MyPass : public FunctionPass
    {
        static char ID;

        MyPass() : FunctionPass(ID)
        {
        }

        std::string LOG_START = "func_start_logger";
        std::string LOGGING = "logging";
        std::string LOG_END = "get_logs";

        virtual bool runOnFunction(Function& F)
        {
            if (F.isDeclaration()) return false;
            if (F.isIntrinsic()) return false;
            const auto name_function = F.getName();
            if (name_function == LOG_START || name_function == LOGGING || name_function == LOG_END) return false;

            const auto module = F.getParent();
            LLVMContext& ctx = module->getContext();
            IRBuilder<> builder(ctx);

            Type* ret_type = Type::getVoidTy(ctx);

            FunctionType* func_start_t = FunctionType::get(ret_type, {builder.getInt8Ty()->getPointerTo()},
                                                           false);
            FunctionType* func_logging_t = FunctionType::get(ret_type, {
                                                                 builder.getInt8Ty()->getPointerTo(),
                                                                 builder.getInt8Ty()->getPointerTo()
                                                             }, false);
            FunctionType* func_end_t = FunctionType::get(ret_type, false);

            const auto log_start = module->getOrInsertFunction(LOG_START, func_start_t);
            const auto logging = module->getOrInsertFunction(LOGGING, func_logging_t);
            const auto log_end = module->getOrInsertFunction(LOG_END, func_end_t);

            builder.SetInsertPoint(&F.getEntryBlock(), F.getEntryBlock().getFirstInsertionPt());
            builder.CreateCall(log_start, {builder.CreateGlobalStringPtr(F.getName())});

            for (auto& B : F)
            {
                for (auto& I : B)
                {
                    if (dyn_cast<PHINode>(&I))
                    {
                        continue;
                    }

                    for (auto& use : I.uses())
                    {
                        if (const auto* user = dyn_cast<Instruction>(use.getUser()))
                        {
                            builder.SetInsertPoint(&I);
                            Value* args[] = {
                                builder.CreateGlobalStringPtr(I.getOpcodeName()),
                                builder.CreateGlobalStringPtr(user->getOpcodeName())

                            };
                            builder.CreateCall(logging, args);
                        }
                    }

                    if (auto* ret = dyn_cast<ReturnInst>(&I))
                    {
                        builder.SetInsertPoint(ret);

                        builder.CreateCall(log_end, {});
                    }
                }
            }

            return false;
        }
    };
}

char MyPass::ID = 0;

static void registerMyPass(const PassManagerBuilder&,
                           legacy::PassManagerBase& PM)
{
    PM.add(new MyPass());
}

static RegisterStandardPasses
RegisterMyPass(PassManagerBuilder::EP_EarlyAsPossible,
               registerMyPass);

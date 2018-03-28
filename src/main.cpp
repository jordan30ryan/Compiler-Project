#include "token.h"
#include "errhandler.h"
#include "symboltable.h"
#include "scanner.h"
#include "parser.h"

#include <vector>
#include <iostream>

#include "llvm/IR/Module.h"

#include <iostream>
#include <cstdint>
#include <cstring>


bool compile(char* filename, ErrHandler* err_handler)
{
    // Remove extension from input filename
    std::string filenamestr(filename);
    int extidx = filenamestr.find(".src");
    if (extidx > 0)
        filenamestr = filenamestr.substr(0, extidx);

    std::cout << "Compiling: " << filenamestr << '\n';

    SymbolTableManager* sym_manager = new SymbolTableManager(err_handler);

    Scanner* scanner = new Scanner(err_handler, sym_manager);
    if (!scanner->init(filename))
    {
        err_handler->reportError("Scanner initialization failed. Ensure the input file is valid.");
        return false;
    }


    // Parse the tokens
    Parser* parser = new Parser(err_handler, sym_manager, scanner, filenamestr);
    std::unique_ptr<llvm::Module> TheModule = parser->parse();

    // Compile the llvm to a file
    compile_to_file(std::move(TheModule));

    // Delete instances
    delete sym_manager;
    delete scanner;
    delete parser;

    // TODO: Compile ll / link with runtime?
    return true;
}

/*
Return codes
1 - No filename given
2 - Some errors reported by err_handler
*/
int main(int argc, char** argv)
{
    /*
    //AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
    // Applies only to this scope
    using namespace llvm;
    using namespace llvm::sys;

    static llvm::LLVMContext TheContext;
    static llvm::IRBuilder<> Builder(TheContext);
    static std::unique_ptr<llvm::Module> TheModule;

    TheModule = make_unique<Module>("my IR", TheContext);

    // Build a simple IR
    // Set up function main (returns i32, no params)
    std::vector<Type *> Parameters;
    FunctionType *FT =
        FunctionType::get(Type::getInt32Ty(TheContext), Parameters, false);
    Function *F =
        Function::Create(FT, Function::ExternalLinkage, "main", TheModule.get());

    // Create basic block of main
    BasicBlock *bb = BasicBlock::Create(TheContext, "entry", F);
    Builder.SetInsertPoint(bb);

    // Return a value
    Value *val = ConstantInt::get(TheContext, APInt(32, 4));
    Builder.CreateRet(val);
    //AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA

    compile_to_file(std::move(TheModule));

    */

    ErrHandler* err_handler = new ErrHandler();

    if (argc < 2) 
    {
        err_handler->reportError("No filename provided.");
        return 1;
    }

    for (int k = 1; k < argc; k++)
    {
        compile(argv[k], err_handler);
    }

    if (err_handler->errors)
    {
        std::cerr << err_handler->errors << " error(s) reported during scanner/parser phase.\n";
        return 2;
    }   

    return 0;
}


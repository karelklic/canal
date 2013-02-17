#include "lib/ArrayStringTrie.h"
#include "lib/Environment.h"
#include "lib/Utils.h"
#include <llvm/Module.h>
#include <llvm/LLVMContext.h>
#include <llvm/Support/ManagedStatic.h>
#include <iostream>

using namespace Canal;

static Environment *gEnvironment;

static llvm::ArrayType *getTestType()
{
    return llvm::ArrayType::get(llvm::Type::getInt8Ty(
        gEnvironment->getContext()), 10);
}

static void testTrieEqualityOperator()
{
    Array::TrieNode trie1 = Array::TrieNode("");
    CANAL_ASSERT(trie1 == trie1);

    Array::TrieNode trie2 = Array::TrieNode("abc");
    Array::TrieNode trie3 = Array::TrieNode("defgh");
    Array::TrieNode trie4 = Array::TrieNode("abc");
    CANAL_ASSERT((trie2 == trie3) == false);
    CANAL_ASSERT(trie2 == trie4);

    Array::TrieNode trie5 = Array::TrieNode("asdf");
    Array::TrieNode *node1 = new Array::TrieNode("qwe");
    trie5.mChildren.push_back(node1);
    CANAL_ASSERT(trie5.mChildren.size() == 1);
    Array::TrieNode trie6 = Array::TrieNode("asdf");
    CANAL_ASSERT(trie6.mChildren.size() == 0);
    CANAL_ASSERT((trie5 == trie6) == false);

    Array::TrieNode trie7 = Array::TrieNode("asdf");
    Array::TrieNode *node2 = new Array::TrieNode("poi");
    trie7.mChildren.push_back(node2);
    CANAL_ASSERT((trie5 == trie7) == false);

    Array::TrieNode trie8 = Array::TrieNode("asdf");
    Array::TrieNode *node3 = new Array::TrieNode("qwe");
    trie8.mChildren.push_back(node3);
    CANAL_ASSERT(trie5 == trie8);
}

static void testConstructors()
{
    const llvm::ArrayType *type = getTestType();
    Array::StringTrie stringTrie(*gEnvironment, *type);
    CANAL_ASSERT(stringTrie.isBottom());
    CANAL_ASSERT(stringTrie.mRoot->mValue == "");
    CANAL_ASSERT(stringTrie.mRoot->mChildren.empty());

    Array::StringTrie stringTrie2(*gEnvironment, "test");
    CANAL_ASSERT(!stringTrie2.isBottom());
    CANAL_ASSERT(stringTrie2.mRoot->mValue == "");
    CANAL_ASSERT(stringTrie2.mRoot->mChildren.size() == 1);
    //std::cout << stringTrie2.mRoot->mChildren[0]->mValue << "\n";
    CANAL_ASSERT(stringTrie2.mRoot->mChildren[0]->mValue == "test");
}

static void testSetTop()
{
    const llvm::ArrayType *type = getTestType();
    Array::StringTrie trie(*gEnvironment, *type);
    trie.setTop();
    CANAL_ASSERT(trie.isTop());
}

static void testSetBottom()
{
    const llvm::ArrayType *type = getTestType();
    Array::StringTrie trie(*gEnvironment, *type);
    trie.setBottom();
    CANAL_ASSERT(trie.isBottom());
}

int
main(int argc, char **argv)
{
    llvm::LLVMContext &context = llvm::getGlobalContext();
    llvm::llvm_shutdown_obj y;

    llvm::Module *module = new llvm::Module("testModule", context);
    gEnvironment = new Environment(module);

    testTrieEqualityOperator();
    testConstructors();
    testSetTop();
    testSetBottom();

    delete gEnvironment;
    return 0;
}


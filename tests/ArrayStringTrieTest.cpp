#include "lib/ArrayStringTrie.h"
#include "lib/ArraySingleItem.h"
#include "lib/Environment.h"
#include "lib/Utils.h"
#include <llvm/Module.h>
#include <llvm/LLVMContext.h>
#include <llvm/Support/ManagedStatic.h>
#include <iostream>

using namespace Canal;

static Environment *gEnvironment;

static llvm::ArrayType *
getTestType()
{
    return llvm::ArrayType::get(llvm::Type::getInt8Ty(
        gEnvironment->getContext()), 10);
}

static void
testTrieCopyConstructor()
{
    Array::TrieNode trie1 = Array::TrieNode("fdsa");
    Array::TrieNode trie2 = Array::TrieNode(trie1);
    CANAL_ASSERT(trie2.mValue == "fdsa");

    Array::TrieNode trie3 = Array::TrieNode("aaa");
    Array::TrieNode *node1 = new Array::TrieNode("bbb");
    trie3.mChildren.insert(node1);
    Array::TrieNode trie4 = Array::TrieNode(trie3);
    CANAL_ASSERT(trie4.mValue == "aaa");
    CANAL_ASSERT(trie4.mChildren.size() == 1);
    Array::TrieNode *node2 = *trie4.mChildren.begin();
    CANAL_ASSERT(node2->mValue == "bbb");
}

static void
testTrieEqualityOperator()
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
    trie5.mChildren.insert(node1);
    CANAL_ASSERT(trie5.mChildren.size() == 1);
    Array::TrieNode trie6 = Array::TrieNode("asdf");
    CANAL_ASSERT(trie6.mChildren.empty());
    CANAL_ASSERT((trie5 == trie6) == false);

    Array::TrieNode trie7 = Array::TrieNode("asdf");
    Array::TrieNode *node2 = new Array::TrieNode("poi");
    trie7.mChildren.insert(node2);
    CANAL_ASSERT((trie5 == trie7) == false);

    Array::TrieNode trie8 = Array::TrieNode("asdf");
    Array::TrieNode *node3 = new Array::TrieNode("qwe");
    trie8.mChildren.insert(node3);
    CANAL_ASSERT(trie5 == trie8);

    Array::TrieNode trie9 = Array::TrieNode("sub");
    Array::TrieNode *node4 = new Array::TrieNode("p");
    Array::TrieNode *node5 = new Array::TrieNode("ex");
    trie9.mChildren.insert(node4);
    trie9.mChildren.insert(node5);

    Array::TrieNode trie10 = Array::TrieNode("sub");
    Array::TrieNode *node6 = new Array::TrieNode("ex");
    Array::TrieNode *node7 = new Array::TrieNode("p");
    trie10.mChildren.insert(node6);
    trie10.mChildren.insert(node7);

    CANAL_ASSERT(trie9 == trie10);

    Array::TrieNode trie11 = Array::TrieNode("sub");
    Array::TrieNode *node8 = new Array::TrieNode("p");
    Array::TrieNode *node9 = new Array::TrieNode("ey");
    trie11.mChildren.insert(node8);
    trie11.mChildren.insert(node9);

    CANAL_ASSERT((trie9 == trie11) == false);
}

static void
testTrieInequalityOperator()
{
    Array::TrieNode trie1 = Array::TrieNode("");
    CANAL_ASSERT((trie1 != trie1) == false);

    Array::TrieNode trie2 = Array::TrieNode("xyz");
    Array::TrieNode trie3 = Array::TrieNode("ijklm");
    Array::TrieNode trie4 = Array::TrieNode("xyz");
    CANAL_ASSERT(trie2 != trie3);
    CANAL_ASSERT((trie2 != trie4) == false);

    Array::TrieNode trie5 = Array::TrieNode("qwer");
    Array::TrieNode *node1 = new Array::TrieNode("zxc");
    trie5.mChildren.insert(node1);
    CANAL_ASSERT(trie5.mChildren.size() == 1);
    Array::TrieNode trie6 = Array::TrieNode("qwer");
    CANAL_ASSERT(trie6.mChildren.empty());
    CANAL_ASSERT(trie5 != trie6);

    Array::TrieNode trie7 = Array::TrieNode("qwer");
    Array::TrieNode *node2 = new Array::TrieNode("abc");
    trie7.mChildren.insert(node2);
    CANAL_ASSERT(trie5 != trie7);

    Array::TrieNode trie8 = Array::TrieNode("qwer");
    Array::TrieNode *node3 = new Array::TrieNode("zxc");
    trie8.mChildren.insert(node3);
    CANAL_ASSERT((trie5 != trie8) == false);
}

static void
testTrieToString()
{
    Array::TrieNode trie1 = Array::TrieNode("");
    CANAL_ASSERT(trie1.toString() == "");

    Array::TrieNode trie2 = Array::TrieNode("asd");
    CANAL_ASSERT(trie2.toString() == "asd");

    Array::TrieNode trie3 = Array::TrieNode("gr");
    Array::TrieNode *node1 = new Array::TrieNode("eat");
    Array::TrieNode *node2 = new Array::TrieNode("ow");
    Array::TrieNode *node3 = new Array::TrieNode("th");
    node2->mChildren.insert(node3);
    trie3.mChildren.insert(node1);
    trie3.mChildren.insert(node2);
    CANAL_ASSERT(trie3.toString() == "gr(eat|ow(th)?)?")
}

static void
testTrieSize()
{
    Array::TrieNode trie1 = Array::TrieNode("");
    CANAL_ASSERT(trie1.size() == 0);

    Array::TrieNode trie2 = Array::TrieNode("test");
    CANAL_ASSERT(trie2.size() == 4);

    Array::TrieNode trie3 = Array::TrieNode("");
    trie3.insert("abc");
    trie3.insert("defgh");
    trie3.insert("ij");
    CANAL_ASSERT(trie3.size() == 10);
}

static void
testTrieGetNumberOfMatchingSymbols()
{
    Array::TrieNode trie1 = Array::TrieNode("testing");
    size_t result1 = trie1.getNumberOfMatchingSymbols("television");
    CANAL_ASSERT(result1 == 2);

    Array::TrieNode trie2 = Array::TrieNode("category");
    size_t result2 = trie2.getNumberOfMatchingSymbols("cat");
    CANAL_ASSERT(result2 == 3);

    Array::TrieNode trie3 = Array::TrieNode("dog");
    size_t result3 = trie3.getNumberOfMatchingSymbols("monkey");
    CANAL_ASSERT(result3 == 0);
}

static void
testTrieGetMatchingChild()
{
    Array::TrieNode trie1 = Array::TrieNode("");
    Array::TrieNode *node1 = new Array::TrieNode("house");
    Array::TrieNode *node2 = new Array::TrieNode("game");
    trie1.mChildren.insert(node1);
    trie1.mChildren.insert(node2);
    Array::TrieNode *result1 = trie1.getMatchingChild("home");
    CANAL_ASSERT(result1 != NULL);
    CANAL_ASSERT(result1->mValue == "house");

    Array::TrieNode trie2 = Array::TrieNode("");
    Array::TrieNode *node3 = new Array::TrieNode("pocket");
    Array::TrieNode *node4 = new Array::TrieNode("hole");
    trie2.mChildren.insert(node3);
    trie2.mChildren.insert(node4);
    Array::TrieNode *result2 = trie2.getMatchingChild("stone");
    CANAL_ASSERT(result2 == NULL);
}

static void
testTrieSplit()
{
    Array::TrieNode trie1 = Array::TrieNode("hello");
    trie1.split(2);
    CANAL_ASSERT(trie1.mValue == "he");
    Array::TrieNode *node1 = *trie1.mChildren.begin();
    CANAL_ASSERT(node1->mValue == "llo");
}

static void
testTrieInsert()
{
    Array::TrieNode trie1 = Array::TrieNode("");
    trie1.insert("test");
    CANAL_ASSERT(trie1.mChildren.size() == 1);
    Array::TrieNode *result1 = *trie1.mChildren.begin();
    CANAL_ASSERT(result1->mValue == "test");

    Array::TrieNode trie2 = Array::TrieNode("");
    trie2.insert("car");
    trie2.insert("house");
    CANAL_ASSERT(trie2.mChildren.size() == 2);
    Array::TrieNode *result2 = *trie2.mChildren.begin();
    CANAL_ASSERT(result2->mValue == "car");
    Array::TrieNode *result3 = *(++trie2.mChildren.begin());
    CANAL_ASSERT(result3->mValue == "house");

    Array::TrieNode trie3 = Array::TrieNode("");
    trie3.insert("home");
    trie3.insert("house");
    CANAL_ASSERT(trie3.mChildren.size() == 1);
    Array::TrieNode *result4 = *trie3.mChildren.begin();
    CANAL_ASSERT(result4->mValue == "ho");
    CANAL_ASSERT(result4->mChildren.size() == 2);
    Array::TrieNode *subresult1 = *result4->mChildren.begin();
    CANAL_ASSERT(subresult1->mValue == "me");
    Array::TrieNode *subresult2 = *(++result4->mChildren.begin());
    CANAL_ASSERT(subresult2->mValue == "use");

    Array::TrieNode trie4 = Array::TrieNode("");
    trie4.insert("good");
    trie4.insert("go");
    CANAL_ASSERT(trie4.mChildren.size() == 1);
    Array::TrieNode *result5 = *trie4.mChildren.begin();
    CANAL_ASSERT(result5->mValue == "go");
    CANAL_ASSERT(result5->mChildren.size() == 1);
    Array::TrieNode *subresult3 = *result5->mChildren.begin();
    CANAL_ASSERT(subresult3->mValue == "od");
}

static void
testTrieGetRepresentedStrings()
{
    Array::TrieNode trie1 = Array::TrieNode("");
    trie1.insert("test");
    std::vector<std::string> results1;
    trie1.getRepresentedStrings(results1, trie1.mValue);
    CANAL_ASSERT(results1.size() == 1);
    CANAL_ASSERT(results1[0] == "test");

    Array::TrieNode trie2 = Array::TrieNode("");
    trie2.insert("car");
    trie2.insert("house");
    std::vector<std::string> results2;
    trie2.getRepresentedStrings(results2, trie2.mValue);
    CANAL_ASSERT(results2.size() == 2);
    CANAL_ASSERT(results2[0] == "car");
    CANAL_ASSERT(results2[1] == "house");

    Array::TrieNode trie3 = Array::TrieNode("");
    trie3.insert("stop");
    trie3.insert("super");
    trie3.insert("step");
    std::vector<std::string> results3;
    trie3.getRepresentedStrings(results3, trie3.mValue);
    CANAL_ASSERT(results3.size() == 3);
    CANAL_ASSERT(results3[0] == "step");
    CANAL_ASSERT(results3[1] == "stop");
    CANAL_ASSERT(results3[2] == "super");
}

static void
testConstructors()
{
    const llvm::ArrayType *type = getTestType();
    Array::StringTrie stringTrie(*gEnvironment, *type);
    CANAL_ASSERT(stringTrie.isBottom());
    CANAL_ASSERT(stringTrie.mRoot->mValue == "");
    CANAL_ASSERT(stringTrie.mRoot->mChildren.empty());

    // TODO test second constructor?

    Array::StringTrie stringTrie2(*gEnvironment, "test");
    CANAL_ASSERT(!stringTrie2.isBottom());
    CANAL_ASSERT(stringTrie2.mRoot->mValue == "");
    CANAL_ASSERT(stringTrie2.mRoot->mChildren.size() == 1);
    Array::TrieNode *node = *stringTrie2.mRoot->mChildren.begin();
    CANAL_ASSERT(node->mValue == "test");
}

static void
testEqualityOperator()
{
    const llvm::ArrayType *type = getTestType();
    
    Array::StringTrie trie1(*gEnvironment, *type);
    CANAL_ASSERT(trie1 == trie1);

    Array::StringTrie trie2(*gEnvironment, *type);
    CANAL_ASSERT(trie1 == trie2);

    Array::StringTrie trie3(*gEnvironment, "test");
    CANAL_ASSERT((trie1 == trie3) == false);

    Array::StringTrie trie4(*gEnvironment, "test");
    CANAL_ASSERT(trie3 == trie4);

    Array::StringTrie trie5(*gEnvironment, "aaa");
    CANAL_ASSERT((trie4 == trie5) == false);

    Array::StringTrie trie6(*gEnvironment, "aaa");
    Array::TrieNode *node1 = new Array::TrieNode("xzy");
    Array::TrieNode *node2 = new Array::TrieNode("bc");
    Array::TrieNode *first1 = *trie6.mRoot->mChildren.begin();
    first1->mChildren.insert(node1);
    first1->mChildren.insert(node2);

    Array::StringTrie trie7(*gEnvironment, "aaa");
    Array::TrieNode *node3 = new Array::TrieNode("bc");
    Array::TrieNode *node4 = new Array::TrieNode("xzy");
    Array::TrieNode *first2 = *trie7.mRoot->mChildren.begin();
    first2->mChildren.insert(node3);
    first2->mChildren.insert(node4);

    CANAL_ASSERT(trie6 == trie7);
    CANAL_ASSERT((trie5 == trie7) == false);
}

static void
testToString()
{
    const llvm::ArrayType *type = getTestType();
    Array::StringTrie trie1(*gEnvironment, *type);
    CANAL_ASSERT(trie1.toString() == "stringTrie bottom\n    type [10 x i8]\n");

    Array::StringTrie trie2(*gEnvironment, *type);
    trie2.setTop();
    CANAL_ASSERT(trie2.toString() == "stringTrie top\n    type [10 x i8]\n");

    Array::StringTrie trie3(*gEnvironment, "asdf");
    CANAL_ASSERT(trie3.toString() ==
    "stringTrie \n    type [4 x i8]\n    (asdf)?\n");
}

static void
testJoin()
{
    const llvm::ArrayType *type = getTestType();

    // bottom vs bottom
    Array::StringTrie test1(*gEnvironment, *type);
    Array::StringTrie bottom(*gEnvironment, *type);
    CANAL_ASSERT(test1.isBottom());
    CANAL_ASSERT(bottom.isBottom());
    test1.join(bottom);
    CANAL_ASSERT(test1.isBottom());

    // bottom vs non-bottom
    Array::StringTrie test2(*gEnvironment, *type);
    Array::StringTrie trie(*gEnvironment, "test");
    CANAL_ASSERT(test2.isBottom());
    CANAL_ASSERT(!trie.isBottom() && !trie.isTop());
    test2.join(trie);
    CANAL_ASSERT(!test2.isBottom());
    Array::TrieNode *node = *test2.mRoot->mChildren.begin();
    CANAL_ASSERT(node->mValue == "test");
    CANAL_ASSERT(!test2.isTop());

    // bottom vs top
    Array::StringTrie test3(*gEnvironment, *type);
    Array::StringTrie top(*gEnvironment, *type);
    top.setTop();
    CANAL_ASSERT(test3.isBottom());
    CANAL_ASSERT(top.isTop());
    test3.join(top);
    CANAL_ASSERT(test3.isTop());

    // value vs bottom
    Array::StringTrie test4(*gEnvironment, "abcdefgh");
    CANAL_ASSERT(!test4.isBottom() && !test4.isTop());
    test4.join(bottom);
    CANAL_ASSERT(!test4.isBottom() && !test4.isTop());
    Array::TrieNode *node2 = *test4.mRoot->mChildren.begin();
    CANAL_ASSERT(node2->mValue == "abcdefgh");

    // value vs value
    Array::StringTrie test5_1(*gEnvironment, "stop");
    Array::StringTrie test5_2(*gEnvironment, "step");
    test5_1.join(test5_2);
    CANAL_ASSERT(test5_1.mRoot->mChildren.size() == 1);
    std::set<Array::TrieNode *, Array::TrieNode::Compare>::const_iterator it5_1 =
        test5_1.mRoot->mChildren.begin();
    Array::TrieNode *node5_1 = *it5_1;
    CANAL_ASSERT(node5_1->mValue == "st");
    CANAL_ASSERT(node5_1->mChildren.size() == 2);
    std::set<Array::TrieNode *, Array::TrieNode::Compare>::const_iterator it5_2 =
        node5_1->mChildren.begin();
    Array::TrieNode *node5_2 = *it5_2;
    Array::TrieNode *node5_3 = *(++it5_2);
    CANAL_ASSERT(node5_2->mValue == "ep");
    CANAL_ASSERT(node5_3->mValue == "op");

    Array::StringTrie test5_3(*gEnvironment, "step");
    Array::StringTrie test5_4(*gEnvironment, "super");
    test5_3.join(test5_4);
    CANAL_ASSERT(test5_3.mRoot->mChildren.size() == 1);
    std::set<Array::TrieNode *, Array::TrieNode::Compare>::const_iterator it5_3 =
        test5_3.mRoot->mChildren.begin();
    Array::TrieNode *node5_4 = *it5_3;
    CANAL_ASSERT(node5_4->mValue == "s");
    CANAL_ASSERT(node5_4->mChildren.size() == 2);
    std::set<Array::TrieNode *, Array::TrieNode::Compare>::const_iterator it5_4 =
        node5_4->mChildren.begin();
    Array::TrieNode *node5_5 = *it5_4;
    Array::TrieNode *node5_6 = *(++it5_4);
    CANAL_ASSERT(node5_5->mValue == "tep");
    CANAL_ASSERT(node5_6->mValue == "uper");

    test5_1.join(test5_3);
    CANAL_ASSERT(test5_1.mRoot->mChildren.size() == 1);
    std::set<Array::TrieNode *, Array::TrieNode::Compare>::const_iterator it5_5 =
        test5_1.mRoot->mChildren.begin();
    Array::TrieNode *node5_7 = *it5_5;
    CANAL_ASSERT(node5_7->mValue == "s");
    //CANAL_ASSERT(node5_7->mChildren.size() == 2);

    // value vs top
    Array::StringTrie test6(*gEnvironment, "qwerty");
    CANAL_ASSERT(!test6.isBottom() && !test6.isTop());
    test6.join(top);
    CANAL_ASSERT(test6.isTop());

    // top vs bottom
    Array::StringTrie test7(*gEnvironment, *type);
    test7.setTop();
    CANAL_ASSERT(test7.isTop());
    test7.join(bottom);
    CANAL_ASSERT(test7.isTop());

    // top vs non-bottom
    Array::StringTrie test8(*gEnvironment, *type);
    test8.setTop();
    CANAL_ASSERT(test8.isTop());
    test8.join(trie);
    CANAL_ASSERT(test8.isTop());

    // top vs top
    Array::StringTrie test9(*gEnvironment, *type);
    test9.setTop();
    CANAL_ASSERT(test9.isTop());
    test9.join(top);
    CANAL_ASSERT(test9.isTop());
}

static void
testSetTop()
{
    const llvm::ArrayType *type = getTestType();
    Array::StringTrie trie(*gEnvironment, *type);
    trie.setTop();
    CANAL_ASSERT(trie.isTop());
}

static void
testSetBottom()
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

    testTrieCopyConstructor();
    testTrieEqualityOperator();
    testTrieInequalityOperator();
    testTrieToString();
    testTrieSize();
    testTrieGetNumberOfMatchingSymbols();
    testTrieGetMatchingChild();
    testTrieSplit();
    testTrieInsert();
    testTrieGetRepresentedStrings();
    testConstructors();
    testEqualityOperator();
    testToString();
    testJoin();
    testSetTop();
    testSetBottom();

    delete gEnvironment;
    return 0;
}


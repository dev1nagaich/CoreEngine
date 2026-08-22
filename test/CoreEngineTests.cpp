#include "../PriceLevel.hpp"
#include "../Ticket.hpp"
#include "../Ledger.hpp"

#include <gtest/gtest.h>
#include <vector>

struct LedgerTests: public ::testing::Test
{
    Ledger* ledger;

    virtual void SetUp() override{
        ledger = new Ledger();
    }

    virtual void TearDown() override{
        delete ledger;
    }
};

TEST_F(LedgerTests, TestBookCreated) {
    EXPECT_NE(ledger, nullptr);
}

// Adding orders tests
TEST_F(LedgerTests, TestAddingAnOrder) {
    EXPECT_EQ(ledger->searchTicketRegistry(357), nullptr);
    EXPECT_EQ(ledger->searchLevelMaps(100, true), nullptr);

    ledger->addLimitTicket(357, true, 27, 100);

    EXPECT_EQ(ledger->searchTicketRegistry(357)->getQuantity(), 27);
    EXPECT_EQ(ledger->searchLevelMaps(100, true)->getAggregateQty(), 27);
    EXPECT_EQ(ledger->searchLevelMaps(20, false), nullptr);

    ledger->addLimitTicket(222, false, 35, 110);

    EXPECT_EQ(ledger->searchLevelMaps(110, false)->getAggregateQty(), 35);
}

TEST_F(LedgerTests, TestMultipleOrdersInALimit){
    ledger->addLimitTicket(5, true, 80, 20);

    EXPECT_EQ(ledger->searchLevelMaps(20, true)->getAggregateQty(), 80);
    EXPECT_EQ(ledger->searchLevelMaps(20, true)->getTicketCount(), 1);

    ledger->addLimitTicket(6, true, 32, 20);

    EXPECT_EQ(ledger->searchLevelMaps(20, true)->getAggregateQty(), 112);
    EXPECT_EQ(ledger->searchLevelMaps(20, true)->getTicketCount(), 2);


    ledger->addLimitTicket(7, true, 111, 20);

    EXPECT_EQ(ledger->searchLevelMaps(20, true)->getAggregateQty(), 223);
    EXPECT_EQ(ledger->searchLevelMaps(20, true)->getTicketCount(), 3);

}

// Cancelling orders tests
TEST_F(LedgerTests, TestCancelOrderLeavingNonEmptyLimit){
    ledger->addLimitTicket(5, true, 80, 20);
    ledger->addLimitTicket(6, true, 32, 20);
    ledger->addLimitTicket(7, true, 111, 20);

    EXPECT_EQ(ledger->searchLevelMaps(20, true)->getTicketCount(), 3);
    EXPECT_EQ(ledger->searchLevelMaps(20, true)->getAggregateQty(), 223);

    ledger->cancelLimitTicket(6);

    EXPECT_EQ(ledger->searchLevelMaps(20, true)->getTicketCount(), 2);
    EXPECT_EQ(ledger->searchLevelMaps(20, true)->getAggregateQty(), 191);

    ledger->cancelLimitTicket(7);

    EXPECT_EQ(ledger->searchLevelMaps(20, true)->getTicketCount(), 1);
    EXPECT_EQ(ledger->searchLevelMaps(20, true)->getAggregateQty(), 80);
}

TEST_F(LedgerTests, TestLimitHeadOrderChangeOnOrderCancel){
    ledger->addLimitTicket(5, true, 80, 20);
    ledger->addLimitTicket(6, true, 32, 20);
    ledger->addLimitTicket(7, true, 111, 20);

    PriceLevel* limit = ledger->searchLevelMaps(20, true);

    EXPECT_EQ(limit->getFrontTicket()->getTicketId(), 5);

    ledger->cancelLimitTicket(5);
    
    EXPECT_EQ(limit->getFrontTicket()->getTicketId(), 6);
}

TEST_F(LedgerTests, TestLimitHeadOrderChangeOnOrderCancelLeavingEmptyLimit){
    ledger->addLimitTicket(5, true, 80, 20);

    PriceLevel* limit = ledger->searchLevelMaps(20, true);

    EXPECT_EQ(limit->getFrontTicket()->getTicketId(), 5);

    ledger->cancelLimitTicket(5);
    
    EXPECT_EQ(limit->getFrontTicket(), nullptr);
}

TEST_F(LedgerTests, TestCancelOrderLeavingEmptyLimit){
    ledger->addLimitTicket(5, true, 80, 20);
    ledger->addLimitTicket(6, true, 80, 15);
    PriceLevel* limit1 = ledger->searchLevelMaps(20, true);
    PriceLevel* limit2 = ledger->searchLevelMaps(15, true);

    EXPECT_EQ(limit2->getFrontTicket()->getTicketId(), 6);
    EXPECT_EQ(limit1->getLeftChild()->getPrice(), 15);

    ledger->cancelLimitTicket(6);
    
    EXPECT_EQ(ledger->searchLevelMaps(15, true), nullptr);
    EXPECT_EQ(limit1->getLeftChild(), nullptr);
}

// Adding to BST tests
TEST_F(LedgerTests, TestCorrectLimitParent){
    ledger->addLimitTicket(5, true, 80, 20);
    ledger->addLimitTicket(6, true, 80, 15);
    ledger->addLimitTicket(7, true, 80, 25);

    PriceLevel* limit1 = ledger->searchLevelMaps(20, true);
    PriceLevel* limit2 = ledger->searchLevelMaps(15, true);
    PriceLevel* limit3 = ledger->searchLevelMaps(25, true);

    EXPECT_EQ(limit1->getParent(), nullptr);
    EXPECT_EQ(limit2->getParent()->getPrice(), 20);
    EXPECT_EQ(limit3->getParent()->getPrice(), 20);
}

TEST_F(LedgerTests, TestCorrectLimitChildren){
    ledger->addLimitTicket(5, true, 80, 20);
    ledger->addLimitTicket(6, true, 80, 15);
    ledger->addLimitTicket(7, true, 80, 25);

    PriceLevel* limit1 = ledger->searchLevelMaps(20, true);
    PriceLevel* limit2 = ledger->searchLevelMaps(15, true);

    EXPECT_EQ(limit1->getLeftChild()->getPrice(), 15);
    EXPECT_EQ(limit1->getRightChild()->getPrice(), 25);
    EXPECT_EQ(limit2->getLeftChild(), nullptr);
    EXPECT_EQ(limit2->getRightChild(), nullptr);
}

TEST_F(LedgerTests, TestTreeHeightsCorrect){
    ledger->addLimitTicket(5, true, 80, 20);
    PriceLevel* limit1 = ledger->searchLevelMaps(20, true);

    EXPECT_EQ(ledger->getLevelHeight(limit1), 1);

    ledger->addLimitTicket(6, true, 80, 15);
    PriceLevel* limit2 = ledger->searchLevelMaps(15, true);

    EXPECT_EQ(ledger->getLevelHeight(limit2), 1);
    EXPECT_EQ(ledger->getLevelHeight(limit1), 2);

    ledger->addLimitTicket(7, true, 80, 25);
    PriceLevel* limit3 = ledger->searchLevelMaps(25, true);

    EXPECT_EQ(ledger->getLevelHeight(limit3), 1);
    EXPECT_EQ(ledger->getLevelHeight(limit1), 2);

    ledger->addLimitTicket(8, true, 80, 10);
    PriceLevel* limit4 = ledger->searchLevelMaps(10, true);

    EXPECT_EQ(ledger->getLevelHeight(limit4), 1);
    EXPECT_EQ(ledger->getLevelHeight(limit2), 2);
    EXPECT_EQ(ledger->getLevelHeight(limit1), 3);

    ledger->addLimitTicket(9, true, 80, 5);
}

TEST_F(LedgerTests, TestBinarySearchTree){
    ledger->addLimitTicket(5, false, 80, 20);
    ledger->addLimitTicket(6, false, 80, 15);
    ledger->addLimitTicket(7, false, 80, 25);
    ledger->addLimitTicket(8, false, 80, 10);
    ledger->addLimitTicket(9, false, 80, 19);

    std::vector<int> expectedInOrder = {10, 15, 19, 20, 25};
    std::vector<int> actualInOrder = ledger->inOrderTreeTraversal(ledger->getAskTree());

    EXPECT_EQ(expectedInOrder, actualInOrder);

    std::vector<int> expectedPreOrder = {20, 15, 10, 19, 25};
    std::vector<int> actualPreOrder = ledger->preOrderTreeTraversal(ledger->getAskTree());

    EXPECT_EQ(expectedPreOrder, actualPreOrder);

    std::vector<int> expectedPostOrder = {10, 19, 15, 25, 20};
    std::vector<int> actualPostOrder = ledger->postOrderTreeTraversal(ledger->getAskTree());

    EXPECT_EQ(expectedPostOrder, actualPostOrder);
}

// Remove from BST tests
TEST_F(LedgerTests, TestRemoveLimitWithNoChildren){
    ledger->addLimitTicket(5, false, 80, 20);
    ledger->addLimitTicket(6, false, 80, 15);
    ledger->addLimitTicket(7, false, 80, 25);
    ledger->addLimitTicket(8, false, 80, 10);
    ledger->addLimitTicket(9, false, 80, 19);

    PriceLevel* limit = ledger->searchLevelMaps(15, false);

    EXPECT_EQ(limit->getRightChild()->getPrice(), 19);

    ledger->cancelLimitTicket(9);

    EXPECT_EQ(limit->getRightChild(), nullptr);
}

TEST_F(LedgerTests, TestRemoveLimitWithLeftChildOnly){
    ledger->addLimitTicket(5, true, 80, 20);
    ledger->addLimitTicket(6, true, 80, 15);
    ledger->addLimitTicket(7, true, 80, 25);
    ledger->addLimitTicket(8, true, 80, 10);

    PriceLevel* limit1 = ledger->searchLevelMaps(20, true);
    PriceLevel* limit2 = ledger->searchLevelMaps(10, true);

    EXPECT_EQ(limit1->getLeftChild()->getPrice(), 15);
    EXPECT_EQ(limit2->getParent()->getPrice(), 15);

    ledger->cancelLimitTicket(6);

    EXPECT_EQ(limit1->getLeftChild()->getPrice(), 10);
    EXPECT_EQ(limit2->getParent()->getPrice(), 20);
}

TEST_F(LedgerTests, TestRemoveLimitWithRightChildOnly){
    ledger->addLimitTicket(5, true, 80, 20);
    ledger->addLimitTicket(6, true, 80, 15);
    ledger->addLimitTicket(7, true, 80, 25);
    ledger->addLimitTicket(8, true, 80, 19);

    PriceLevel* limit1 = ledger->searchLevelMaps(20, true);
    PriceLevel* limit2 = ledger->searchLevelMaps(19, true);

    EXPECT_EQ(limit1->getLeftChild()->getPrice(), 15);
    EXPECT_EQ(limit2->getParent()->getPrice(), 15);

    ledger->cancelLimitTicket(6);

    EXPECT_EQ(limit1->getLeftChild()->getPrice(), 19);
    EXPECT_EQ(limit2->getParent()->getPrice(), 20);
}

TEST_F(LedgerTests, TestRemoveLimitWithTwoChildren){
    ledger->addLimitTicket(5, true, 80, 20);
    ledger->addLimitTicket(6, true, 80, 15);
    ledger->addLimitTicket(7, true, 80, 25);
    ledger->addLimitTicket(8, true, 80, 10);
    ledger->addLimitTicket(9, true, 80, 18);
    ledger->addLimitTicket(10, true, 80, 23);
    ledger->addLimitTicket(11, true, 80, 27);
    ledger->addLimitTicket(12, true, 80, 17);
    ledger->addLimitTicket(13, true, 80, 19);

    PriceLevel* limit1 = ledger->searchLevelMaps(20, true);
    PriceLevel* limit2 = ledger->searchLevelMaps(10, true);
    PriceLevel* limit3 = ledger->searchLevelMaps(18, true);
    PriceLevel* limit4 = ledger->searchLevelMaps(17, true);

    EXPECT_EQ(limit1->getLeftChild()->getPrice(), 15);
    EXPECT_EQ(limit2->getParent()->getPrice(), 15);
    EXPECT_EQ(limit3->getLeftChild()->getPrice(), 17);

    ledger->cancelLimitTicket(6);

    EXPECT_EQ(limit1->getLeftChild()->getPrice(), 17);
    EXPECT_EQ(limit2->getParent()->getPrice(), 17);
    EXPECT_EQ(limit3->getLeftChild(), nullptr);
    EXPECT_EQ(limit4->getLeftChild()->getPrice(), 10);
    EXPECT_EQ(limit4->getRightChild()->getPrice(), 18);
    EXPECT_EQ(limit4->getParent()->getPrice(), 20);
}

TEST_F(LedgerTests, TestRemoveLimitWithTwoChildren_RightChildHasNoLeftChild){
    ledger->addLimitTicket(5, true, 80, 20);
    ledger->addLimitTicket(6, true, 80, 15);
    ledger->addLimitTicket(7, true, 80, 25);
    ledger->addLimitTicket(8, true, 80, 10);
    ledger->addLimitTicket(9, true, 80, 18);
    ledger->addLimitTicket(10, true, 80, 23);
    ledger->addLimitTicket(11, true, 80, 27);

    PriceLevel* limit1 = ledger->searchLevelMaps(20, true);
    PriceLevel* limit2 = ledger->searchLevelMaps(23, true);
    PriceLevel* limit3 = ledger->searchLevelMaps(27, true);

    EXPECT_EQ(limit1->getRightChild()->getPrice(), 25);
    EXPECT_EQ(limit2->getParent()->getPrice(), 25);
    EXPECT_EQ(limit3->getParent()->getPrice(), 25);

    ledger->cancelLimitTicket(7);

    EXPECT_EQ(limit1->getRightChild()->getPrice(), 27);
    EXPECT_EQ(limit2->getParent()->getPrice(), 27);
    EXPECT_EQ(limit3->getParent()->getPrice(), 20);
}

TEST_F(LedgerTests, TestRemoveLimitWithTwoChildren_RightChildHasLeftChildWithRightChild){
    ledger->addLimitTicket(3, true, 80, 224);
    ledger->addLimitTicket(4, true, 80, 220);
    ledger->addLimitTicket(5, true, 80, 228);
    ledger->addLimitTicket(6, true, 80, 218);
    ledger->addLimitTicket(7, true, 80, 221);
    ledger->addLimitTicket(8, true, 80, 226);
    ledger->addLimitTicket(9, true, 80, 231);
    ledger->addLimitTicket(10, true, 80, 217);
    ledger->addLimitTicket(11, true, 80, 225);
    ledger->addLimitTicket(12, true, 80, 229);
    ledger->addLimitTicket(13, true, 80, 233);
    ledger->addLimitTicket(14, true, 80, 230);

    std::vector<int> expectedInOrder = {217, 218, 220, 221, 224, 225, 226, 228, 229, 230, 231, 233};
    std::vector<int> actualInOrder = ledger->inOrderTreeTraversal(ledger->getBidTree());

    EXPECT_EQ(expectedInOrder, actualInOrder);

    std::vector<int> expectedPreOrder = {224, 220, 218, 217, 221, 228, 226, 225, 231, 229, 230, 233};
    std::vector<int> actualPreOrder = ledger->preOrderTreeTraversal(ledger->getBidTree());

    EXPECT_EQ(expectedPreOrder, actualPreOrder);

    std::vector<int> expectedPostOrder = {217, 218, 221, 220, 225, 226, 230, 229, 233, 231, 228, 224};
    std::vector<int> actualPostOrder = ledger->postOrderTreeTraversal(ledger->getBidTree());

    EXPECT_EQ(expectedPostOrder, actualPostOrder);

    ledger->cancelLimitTicket(5);

    expectedInOrder = {217, 218, 220, 221, 224, 225, 226, 229, 230, 231, 233};
    actualInOrder = ledger->inOrderTreeTraversal(ledger->getBidTree());

    EXPECT_EQ(expectedInOrder, actualInOrder);

    expectedPreOrder = {224, 220, 218, 217, 221, 229, 226, 225, 231, 230, 233};
    actualPreOrder = ledger->preOrderTreeTraversal(ledger->getBidTree());

    EXPECT_EQ(expectedPreOrder, actualPreOrder);

    expectedPostOrder = {217, 218, 221, 220, 225, 226, 230, 233, 231, 229, 224};
    actualPostOrder = ledger->postOrderTreeTraversal(ledger->getBidTree());

    EXPECT_EQ(expectedPostOrder, actualPostOrder);
}

TEST_F(LedgerTests, TestRemoveLimitWithTwoChildren_RightChildHasLeftChildWithRightChild2){
    ledger->addLimitTicket(3, true, 80, 250);
    ledger->addLimitTicket(4, true, 80, 255);
    ledger->addLimitTicket(5, true, 80, 228);
    ledger->addLimitTicket(6, true, 80, 251);
    ledger->addLimitTicket(7, true, 80, 260);
    ledger->addLimitTicket(8, true, 80, 226);
    ledger->addLimitTicket(9, true, 80, 231);
    ledger->addLimitTicket(10, true, 80, 265);
    ledger->addLimitTicket(11, true, 80, 225);
    ledger->addLimitTicket(12, true, 80, 229);
    ledger->addLimitTicket(13, true, 80, 233);
    ledger->addLimitTicket(14, true, 80, 230);

    std::vector<int> expectedInOrder = {225, 226, 228, 229, 230, 231, 233, 250, 251, 255, 260, 265};
    std::vector<int> actualInOrder = ledger->inOrderTreeTraversal(ledger->getBidTree());

    EXPECT_EQ(expectedInOrder, actualInOrder);

    std::vector<int> expectedPreOrder = {250, 228, 226, 225, 231, 229, 230, 233, 255, 251, 260, 265};
    std::vector<int> actualPreOrder = ledger->preOrderTreeTraversal(ledger->getBidTree());

    EXPECT_EQ(expectedPreOrder, actualPreOrder);

    std::vector<int> expectedPostOrder = {225, 226, 230, 229, 233, 231, 228, 251, 265, 260, 255, 250};
    std::vector<int> actualPostOrder = ledger->postOrderTreeTraversal(ledger->getBidTree());

    EXPECT_EQ(expectedPostOrder, actualPostOrder);

    ledger->cancelLimitTicket(5);

    expectedInOrder = {225, 226, 229, 230, 231, 233, 250, 251, 255, 260, 265};
    actualInOrder = ledger->inOrderTreeTraversal(ledger->getBidTree());

    EXPECT_EQ(expectedInOrder, actualInOrder);

    expectedPreOrder = {250, 229, 226, 225, 231, 230, 233, 255, 251, 260, 265};
    actualPreOrder = ledger->preOrderTreeTraversal(ledger->getBidTree());

    EXPECT_EQ(expectedPreOrder, actualPreOrder);

    expectedPostOrder = {225, 226, 230, 233, 231, 229, 251, 265, 260, 255, 250};
    actualPostOrder = ledger->postOrderTreeTraversal(ledger->getBidTree());

    EXPECT_EQ(expectedPostOrder, actualPostOrder);
}

TEST_F(LedgerTests, TestEmptyingATree){
    ledger->addLimitTicket(5, true, 80, 20);

    EXPECT_EQ(ledger->getBidTree()->getPrice(), 20);

    ledger->cancelLimitTicket(5);

    EXPECT_EQ(ledger->getBidTree(), nullptr);
}

TEST_F(LedgerTests, TestRemoveRootLimitWithLeftChildOnly){
    ledger->addLimitTicket(5, false, 80, 20);
    ledger->addLimitTicket(6, false, 80, 15);

    PriceLevel* limit = ledger->searchLevelMaps(15, false);

    EXPECT_EQ(ledger->getAskTree()->getPrice(), 20);
    EXPECT_EQ(limit->getParent()->getPrice(), 20);

    ledger->cancelLimitTicket(5);

    EXPECT_EQ(ledger->getAskTree()->getPrice(), 15);
    EXPECT_EQ(limit->getParent(), nullptr);
}

TEST_F(LedgerTests, TestRemoveRootLimitWithRightChildOnly){
    ledger->addLimitTicket(5, true, 80, 20);
    ledger->addLimitTicket(6, true, 80, 25);

    PriceLevel* limit = ledger->searchLevelMaps(25, true);

    EXPECT_EQ(ledger->getBidTree()->getPrice(), 20);
    EXPECT_EQ(limit->getParent()->getPrice(), 20);

    ledger->cancelLimitTicket(5);

    EXPECT_EQ(ledger->getBidTree()->getPrice(), 25);
    EXPECT_EQ(limit->getParent(), nullptr);
}

TEST_F(LedgerTests, TestRemoveRootLimitWithTwoChildren){
    ledger->addLimitTicket(5, true, 80, 20);
    ledger->addLimitTicket(6, true, 80, 15);
    ledger->addLimitTicket(7, true, 80, 25);
    ledger->addLimitTicket(8, true, 80, 27);
    ledger->addLimitTicket(9, true, 80, 22);

    PriceLevel* limit1 = ledger->searchLevelMaps(15, true);
    PriceLevel* limit2 = ledger->searchLevelMaps(25, true);
    PriceLevel* limit3 = ledger->searchLevelMaps(22, true);

    EXPECT_EQ(ledger->getBidTree()->getPrice(), 20);
    EXPECT_EQ(limit1->getParent()->getPrice(), 20);
    EXPECT_EQ(limit2->getParent()->getPrice(), 20);
    EXPECT_EQ(limit2->getLeftChild()->getPrice(), 22);
    EXPECT_EQ(limit3->getParent()->getPrice(), 25);

    ledger->cancelLimitTicket(5);

    EXPECT_EQ(ledger->getBidTree()->getPrice(), 22);
    EXPECT_EQ(limit1->getParent()->getPrice(), 22);
    EXPECT_EQ(limit2->getParent()->getPrice(), 22);
    EXPECT_EQ(limit2->getLeftChild(), nullptr);
    EXPECT_EQ(limit3->getParent(), nullptr);
}

TEST_F(LedgerTests, TestRemoveRootLimitWithTwoChildren_RightChildHasNoLeftChild){
    ledger->addLimitTicket(5, true, 80, 20);
    ledger->addLimitTicket(6, true, 80, 15);
    ledger->addLimitTicket(7, true, 80, 25);
    ledger->addLimitTicket(8, true, 80, 27);

    PriceLevel* limit1 = ledger->searchLevelMaps(15, true);
    PriceLevel* limit2 = ledger->searchLevelMaps(25, true);

    EXPECT_EQ(ledger->getBidTree()->getPrice(), 20);
    EXPECT_EQ(limit1->getParent()->getPrice(), 20);
    EXPECT_EQ(limit2->getParent()->getPrice(), 20);

    ledger->cancelLimitTicket(5);

    EXPECT_EQ(ledger->getBidTree()->getPrice(), 25);
    EXPECT_EQ(limit1->getParent()->getPrice(), 25);
    EXPECT_EQ(limit2->getParent(), nullptr);
}

TEST_F(LedgerTests, TestRemoveRootLimitWithTwoChildren_RightChildHasLeftChildWithRightChild){
    ledger->addLimitTicket(5, true, 80, 228);
    ledger->addLimitTicket(6, true, 80, 226);
    ledger->addLimitTicket(7, true, 80, 231);
    ledger->addLimitTicket(8, true, 80, 225);
    ledger->addLimitTicket(9, true, 80, 229);
    ledger->addLimitTicket(10, true, 80, 233);
    ledger->addLimitTicket(11, true, 80, 230);

    std::vector<int> expectedInOrder = {225, 226, 228, 229, 230, 231, 233};
    std::vector<int> actualInOrder = ledger->inOrderTreeTraversal(ledger->getBidTree());

    EXPECT_EQ(expectedInOrder, actualInOrder);

    std::vector<int> expectedPreOrder = {228, 226, 225, 231, 229, 230, 233};
    std::vector<int> actualPreOrder = ledger->preOrderTreeTraversal(ledger->getBidTree());

    EXPECT_EQ(expectedPreOrder, actualPreOrder);

    std::vector<int> expectedPostOrder = {225, 226, 230, 229, 233, 231, 228};
    std::vector<int> actualPostOrder = ledger->postOrderTreeTraversal(ledger->getBidTree());

    EXPECT_EQ(expectedPostOrder, actualPostOrder);

    ledger->cancelLimitTicket(5);

    expectedInOrder = {225, 226, 229, 230, 231, 233};
    actualInOrder = ledger->inOrderTreeTraversal(ledger->getBidTree());

    EXPECT_EQ(expectedInOrder, actualInOrder);

    expectedPreOrder = {229, 226, 225, 231, 230, 233};
    actualPreOrder = ledger->preOrderTreeTraversal(ledger->getBidTree());

    EXPECT_EQ(expectedPreOrder, actualPreOrder);

    expectedPostOrder = {225, 226, 230, 233, 231, 229};
    actualPostOrder = ledger->postOrderTreeTraversal(ledger->getBidTree());

    EXPECT_EQ(expectedPostOrder, actualPostOrder);
}

// AVL tree balancing tests
TEST_F(LedgerTests, TestAVLTreeRRRotateOnInsert){
    ledger->addLimitTicket(5, true, 80, 20);
    ledger->addLimitTicket(6, true, 80, 15);
    ledger->addLimitTicket(7, true, 80, 25);
    ledger->addLimitTicket(8, true, 80, 10);
    ledger->addLimitTicket(9, true, 80, 17);
    ledger->addLimitTicket(10, true, 80, 30);

    PriceLevel* limit1 = ledger->searchLevelMaps(20, true);
    PriceLevel* limit2 = ledger->searchLevelMaps(25, true);
    PriceLevel* limit3 = ledger->searchLevelMaps(30, true);

    EXPECT_EQ(limit1->getRightChild()->getPrice(), 25);
    EXPECT_EQ(limit2->getParent()->getPrice(), 20);
    EXPECT_EQ(limit2->getRightChild()->getPrice(), 30);
    EXPECT_EQ(limit3->getParent()->getPrice(), 25);

    ledger->addLimitTicket(11, true, 80, 35);

    PriceLevel* limit4 = ledger->searchLevelMaps(35, true);

    EXPECT_EQ(limit1->getRightChild()->getPrice(), 30);
    EXPECT_EQ(limit2->getParent()->getPrice(), 30);
    EXPECT_EQ(limit2->getRightChild(), nullptr);
    EXPECT_EQ(limit3->getParent()->getPrice(), 20);
    EXPECT_EQ(limit3->getLeftChild()->getPrice(), 25);
    EXPECT_EQ(limit3->getRightChild()->getPrice(), 35);
    EXPECT_EQ(limit4->getParent()->getPrice(), 30);

    std::vector<int> expectedInOrder = {10, 15, 17, 20, 25, 30, 35};
    std::vector<int> actualInOrder = ledger->inOrderTreeTraversal(ledger->getBidTree());

    EXPECT_EQ(expectedInOrder, actualInOrder);

    std::vector<int> expectedPreOrder = {20, 15, 10, 17, 30, 25, 35};
    std::vector<int> actualPreOrder = ledger->preOrderTreeTraversal(ledger->getBidTree());

    EXPECT_EQ(expectedPreOrder, actualPreOrder);

    std::vector<int> expectedPostOrder = {10, 17, 15, 25, 35, 30, 20};
    std::vector<int> actualPostOrder = ledger->postOrderTreeTraversal(ledger->getBidTree());

    EXPECT_EQ(expectedPostOrder, actualPostOrder);
}

TEST_F(LedgerTests, TestAVLTreeLLRotateOnInsert){
    ledger->addLimitTicket(5, true, 80, 20);
    ledger->addLimitTicket(6, true, 80, 15);
    ledger->addLimitTicket(7, true, 80, 25);
    ledger->addLimitTicket(8, true, 80, 10);
    ledger->addLimitTicket(9, true, 80, 22);
    ledger->addLimitTicket(10, true, 80, 30);

    PriceLevel* limit1 = ledger->searchLevelMaps(20, true);
    PriceLevel* limit2 = ledger->searchLevelMaps(15, true);
    PriceLevel* limit3 = ledger->searchLevelMaps(10, true);

    EXPECT_EQ(limit1->getLeftChild()->getPrice(), 15);
    EXPECT_EQ(limit2->getParent()->getPrice(), 20);
    EXPECT_EQ(limit2->getLeftChild()->getPrice(), 10);
    EXPECT_EQ(limit3->getParent()->getPrice(), 15);

    ledger->addLimitTicket(11, true, 80, 5);

    PriceLevel* limit4 = ledger->searchLevelMaps(5, true);

    EXPECT_EQ(limit1->getLeftChild()->getPrice(), 10);
    EXPECT_EQ(limit2->getParent()->getPrice(), 10);
    EXPECT_EQ(limit2->getLeftChild(), nullptr);
    EXPECT_EQ(limit3->getParent()->getPrice(), 20);
    EXPECT_EQ(limit3->getRightChild()->getPrice(), 15);
    EXPECT_EQ(limit3->getLeftChild()->getPrice(), 5);
    EXPECT_EQ(limit4->getParent()->getPrice(), 10);

    std::vector<int> expectedInOrder = {5, 10, 15, 20, 22, 25, 30};
    std::vector<int> actualInOrder = ledger->inOrderTreeTraversal(ledger->getBidTree());

    EXPECT_EQ(expectedInOrder, actualInOrder);

    std::vector<int> expectedPreOrder = {20, 10, 5, 15, 25, 22, 30};
    std::vector<int> actualPreOrder = ledger->preOrderTreeTraversal(ledger->getBidTree());

    EXPECT_EQ(expectedPreOrder, actualPreOrder);

    std::vector<int> expectedPostOrder = {5, 15, 10, 22, 30, 25, 20};
    std::vector<int> actualPostOrder = ledger->postOrderTreeTraversal(ledger->getBidTree());

    EXPECT_EQ(expectedPostOrder, actualPostOrder);
}

TEST_F(LedgerTests, TestAVLTreeRLRotateOnInsert){
    ledger->addLimitTicket(5, true, 80, 20);
    ledger->addLimitTicket(6, true, 80, 15);
    ledger->addLimitTicket(7, true, 80, 25);
    ledger->addLimitTicket(8, true, 80, 10);
    ledger->addLimitTicket(9, true, 80, 17);
    ledger->addLimitTicket(10, true, 80, 24);
    ledger->addLimitTicket(11, true, 80, 30);
    ledger->addLimitTicket(12, true, 80, 5);
    ledger->addLimitTicket(13, true, 80, 28);
    ledger->addLimitTicket(14, true, 80, 35);

    ledger->addLimitTicket(15, true, 80, 26);

    std::vector<int> expectedInOrder = {5, 10, 15, 17, 20, 24, 25, 26, 28, 30, 35};
    std::vector<int> actualInOrder = ledger->inOrderTreeTraversal(ledger->getBidTree());

    EXPECT_EQ(expectedInOrder, actualInOrder);

    std::vector<int> expectedPreOrder = {20, 15, 10, 5, 17, 28, 25, 24, 26, 30, 35};
    std::vector<int> actualPreOrder = ledger->preOrderTreeTraversal(ledger->getBidTree());

    EXPECT_EQ(expectedPreOrder, actualPreOrder);

    std::vector<int> expectedPostOrder = {5, 10, 17, 15, 24, 26, 25, 35, 30, 28, 20};
    std::vector<int> actualPostOrder = ledger->postOrderTreeTraversal(ledger->getBidTree());

    EXPECT_EQ(expectedPostOrder, actualPostOrder);
}

TEST_F(LedgerTests, TestAVLTreeLRRotateOnInsert){
    ledger->addLimitTicket(5, true, 80, 20);
    ledger->addLimitTicket(6, true, 80, 15);
    ledger->addLimitTicket(7, true, 80, 25);
    ledger->addLimitTicket(8, true, 80, 10);
    ledger->addLimitTicket(9, true, 80, 17);
    ledger->addLimitTicket(10, true, 80, 24);
    ledger->addLimitTicket(11, true, 80, 30);
    ledger->addLimitTicket(12, true, 80, 5);
    ledger->addLimitTicket(13, true, 80, 13);
    ledger->addLimitTicket(14, true, 80, 35);

    ledger->addLimitTicket(15, true, 80, 12);

    std::vector<int> expectedInOrder = {5, 10, 12, 13, 15, 17, 20, 24, 25, 30, 35};
    std::vector<int> actualInOrder = ledger->inOrderTreeTraversal(ledger->getBidTree());

    EXPECT_EQ(expectedInOrder, actualInOrder);

    std::vector<int> expectedPreOrder = {20, 13, 10, 5, 12, 15, 17, 25, 24, 30, 35};
    std::vector<int> actualPreOrder = ledger->preOrderTreeTraversal(ledger->getBidTree());

    EXPECT_EQ(expectedPreOrder, actualPreOrder);

    std::vector<int> expectedPostOrder = {5, 12, 10, 17, 15, 13, 24, 35, 30, 25, 20};
    std::vector<int> actualPostOrder = ledger->postOrderTreeTraversal(ledger->getBidTree());

    EXPECT_EQ(expectedPostOrder, actualPostOrder);
}

TEST_F(LedgerTests, TestAVLTreeRRRotateRootOnInsert){
    ledger->addLimitTicket(111, false, 43, 80);
    ledger->addLimitTicket(112, false, 543, 81);
    ledger->addLimitTicket(113, false, 46, 82);

    std::vector<int> expectedInOrder = {80, 81, 82};
    std::vector<int> actualInOrder = ledger->inOrderTreeTraversal(ledger->getAskTree());

    EXPECT_EQ(expectedInOrder, actualInOrder);

    std::vector<int> expectedPreOrder = {81, 80, 82};
    std::vector<int> actualPreOrder = ledger->preOrderTreeTraversal(ledger->getAskTree());

    EXPECT_EQ(expectedPreOrder, actualPreOrder);

    std::vector<int> expectedPostOrder = {80, 82, 81};
    std::vector<int> actualPostOrder = ledger->postOrderTreeTraversal(ledger->getAskTree());

    EXPECT_EQ(expectedPostOrder, actualPostOrder);

    EXPECT_EQ(ledger->getAskTree()->getPrice(), 81);
    EXPECT_EQ(ledger->getAskTree()->getParent(), nullptr);
}

TEST_F(LedgerTests, TestAVLTreeLLRotateRootOnInsert){
    ledger->addLimitTicket(111, true, 43, 80);
    ledger->addLimitTicket(112, true, 543, 79);
    ledger->addLimitTicket(113, true, 46, 78);

    std::vector<int> expectedInOrder = {78, 79, 80};
    std::vector<int> actualInOrder = ledger->inOrderTreeTraversal(ledger->getBidTree());

    EXPECT_EQ(expectedInOrder, actualInOrder);

    std::vector<int> expectedPreOrder = {79, 78, 80};
    std::vector<int> actualPreOrder = ledger->preOrderTreeTraversal(ledger->getBidTree());

    EXPECT_EQ(expectedPreOrder, actualPreOrder);

    std::vector<int> expectedPostOrder = {78, 80, 79};
    std::vector<int> actualPostOrder = ledger->postOrderTreeTraversal(ledger->getBidTree());

    EXPECT_EQ(expectedPostOrder, actualPostOrder);

    EXPECT_EQ(ledger->getBidTree()->getPrice(), 79);
    EXPECT_EQ(ledger->getBidTree()->getParent(), nullptr);
}

TEST_F(LedgerTests, TestAVLTreeRLRotateRootOnInsert){
    ledger->addLimitTicket(111, false, 43, 80);
    ledger->addLimitTicket(112, false, 543, 81);
    ledger->addLimitTicket(113, false, 46, 82);

    std::vector<int> expectedInOrder = {80, 81, 82};
    std::vector<int> actualInOrder = ledger->inOrderTreeTraversal(ledger->getAskTree());

    EXPECT_EQ(expectedInOrder, actualInOrder);

    std::vector<int> expectedPreOrder = {81, 80, 82};
    std::vector<int> actualPreOrder = ledger->preOrderTreeTraversal(ledger->getAskTree());

    EXPECT_EQ(expectedPreOrder, actualPreOrder);

    std::vector<int> expectedPostOrder = {80, 82, 81};
    std::vector<int> actualPostOrder = ledger->postOrderTreeTraversal(ledger->getAskTree());

    EXPECT_EQ(expectedPostOrder, actualPostOrder);

    EXPECT_EQ(ledger->getAskTree()->getPrice(), 81);
    EXPECT_EQ(ledger->getAskTree()->getParent(), nullptr);
}

TEST_F(LedgerTests, TestAVLTreeLRRotateRootOnInsert){
    ledger->addLimitTicket(111, true, 43, 80);
    ledger->addLimitTicket(113, true, 46, 78);
    ledger->addLimitTicket(112, true, 543, 79);

    std::vector<int> expectedInOrder = {78, 79, 80};
    std::vector<int> actualInOrder = ledger->inOrderTreeTraversal(ledger->getBidTree());

    EXPECT_EQ(expectedInOrder, actualInOrder);

    std::vector<int> expectedPreOrder = {79, 78, 80};
    std::vector<int> actualPreOrder = ledger->preOrderTreeTraversal(ledger->getBidTree());

    EXPECT_EQ(expectedPreOrder, actualPreOrder);

    std::vector<int> expectedPostOrder = {78, 80, 79};
    std::vector<int> actualPostOrder = ledger->postOrderTreeTraversal(ledger->getBidTree());

    EXPECT_EQ(expectedPostOrder, actualPostOrder);

    EXPECT_EQ(ledger->getBidTree()->getPrice(), 79);
    EXPECT_EQ(ledger->getBidTree()->getParent(), nullptr);
}

TEST_F(LedgerTests, TestAVLTreeRRRotateOnDelete){
    ledger->addLimitTicket(5, true, 80, 20);
    ledger->addLimitTicket(6, true, 80, 15);
    ledger->addLimitTicket(7, true, 80, 25);
    ledger->addLimitTicket(8, true, 80, 10);
    ledger->addLimitTicket(9, true, 80, 17);
    ledger->addLimitTicket(10, true, 80, 22);
    ledger->addLimitTicket(11, true, 80, 30);
    ledger->addLimitTicket(12, true, 80, 27);
    ledger->addLimitTicket(13, true, 80, 35);


    PriceLevel* limit1 = ledger->searchLevelMaps(20, true);
    PriceLevel* limit2 = ledger->searchLevelMaps(25, true);
    PriceLevel* limit3 = ledger->searchLevelMaps(30, true);
    PriceLevel* limit4 = ledger->searchLevelMaps(35, true);
    PriceLevel* limit5 = ledger->searchLevelMaps(27, true);
    

    EXPECT_EQ(limit1->getRightChild()->getPrice(), 25);
    EXPECT_EQ(limit2->getParent()->getPrice(), 20);
    EXPECT_EQ(limit2->getRightChild()->getPrice(), 30);
    EXPECT_EQ(limit2->getLeftChild()->getPrice(), 22);
    EXPECT_EQ(limit3->getParent()->getPrice(), 25);
    EXPECT_EQ(limit3->getLeftChild()->getPrice(), 27);
    EXPECT_EQ(limit3->getRightChild()->getPrice(), 35);
    EXPECT_EQ(limit4->getParent()->getPrice(), 30);
    EXPECT_EQ(limit5->getParent()->getPrice(), 30);


    ledger->cancelLimitTicket(10);

    EXPECT_EQ(limit1->getRightChild()->getPrice(), 30);
    EXPECT_EQ(limit2->getParent()->getPrice(), 30);
    EXPECT_EQ(limit2->getLeftChild(), nullptr);
    EXPECT_EQ(limit2->getRightChild()->getPrice(), 27);
    EXPECT_EQ(limit3->getParent()->getPrice(), 20);
    EXPECT_EQ(limit3->getLeftChild()->getPrice(), 25);
    EXPECT_EQ(limit3->getRightChild()->getPrice(), 35);
    EXPECT_EQ(limit4->getParent()->getPrice(), 30);
    EXPECT_EQ(limit5->getParent()->getPrice(), 25);

    std::vector<int> expectedInOrder = {10, 15, 17, 20, 25, 27, 30, 35};
    std::vector<int> actualInOrder = ledger->inOrderTreeTraversal(ledger->getBidTree());

    EXPECT_EQ(expectedInOrder, actualInOrder);

    std::vector<int> expectedPreOrder = {20, 15, 10, 17, 30, 25, 27, 35};
    std::vector<int> actualPreOrder = ledger->preOrderTreeTraversal(ledger->getBidTree());

    EXPECT_EQ(expectedPreOrder, actualPreOrder);

    std::vector<int> expectedPostOrder = {10, 17, 15, 27, 25, 35, 30, 20};
    std::vector<int> actualPostOrder = ledger->postOrderTreeTraversal(ledger->getBidTree());

    EXPECT_EQ(expectedPostOrder, actualPostOrder);
}

TEST_F(LedgerTests, TestAVLTreeLLRotateOnDelete){
    ledger->addLimitTicket(5, true, 80, 20);
    ledger->addLimitTicket(6, true, 80, 15);
    ledger->addLimitTicket(7, true, 80, 25);
    ledger->addLimitTicket(8, true, 80, 10);
    ledger->addLimitTicket(9, true, 80, 17);
    ledger->addLimitTicket(10, true, 80, 22);
    ledger->addLimitTicket(11, true, 80, 30);
    ledger->addLimitTicket(12, true, 80, 5);
    ledger->addLimitTicket(13, true, 80, 13);

    PriceLevel* limit1 = ledger->searchLevelMaps(20, true);
    PriceLevel* limit2 = ledger->searchLevelMaps(15, true);
    PriceLevel* limit3 = ledger->searchLevelMaps(10, true);
    PriceLevel* limit4 = ledger->searchLevelMaps(5, true);
    PriceLevel* limit5 = ledger->searchLevelMaps(13, true);

    EXPECT_EQ(limit1->getLeftChild()->getPrice(), 15);
    EXPECT_EQ(limit2->getParent()->getPrice(), 20);
    EXPECT_EQ(limit2->getLeftChild()->getPrice(), 10);
    EXPECT_EQ(limit2->getRightChild()->getPrice(), 17);
    EXPECT_EQ(limit3->getParent()->getPrice(), 15);
    EXPECT_EQ(limit3->getRightChild()->getPrice(), 13);
    EXPECT_EQ(limit3->getLeftChild()->getPrice(), 5);
    EXPECT_EQ(limit4->getParent()->getPrice(), 10);
    EXPECT_EQ(limit5->getParent()->getPrice(), 10);
    
    ledger->cancelLimitTicket(9);

    EXPECT_EQ(limit1->getLeftChild()->getPrice(), 10);
    EXPECT_EQ(limit2->getParent()->getPrice(), 10);
    EXPECT_EQ(limit2->getRightChild(), nullptr);
    EXPECT_EQ(limit2->getLeftChild()->getPrice(), 13);
    EXPECT_EQ(limit3->getParent()->getPrice(), 20);
    EXPECT_EQ(limit3->getRightChild()->getPrice(), 15);
    EXPECT_EQ(limit3->getLeftChild()->getPrice(), 5);
    EXPECT_EQ(limit4->getParent()->getPrice(), 10);
    EXPECT_EQ(limit5->getParent()->getPrice(), 15);

    std::vector<int> expectedInOrder = {5, 10, 13, 15, 20, 22, 25, 30};
    std::vector<int> actualInOrder = ledger->inOrderTreeTraversal(ledger->getBidTree());

    EXPECT_EQ(expectedInOrder, actualInOrder);

    std::vector<int> expectedPreOrder = {20, 10, 5, 15, 13, 25, 22, 30};
    std::vector<int> actualPreOrder = ledger->preOrderTreeTraversal(ledger->getBidTree());

    EXPECT_EQ(expectedPreOrder, actualPreOrder);

    std::vector<int> expectedPostOrder = {5, 13, 15, 10, 22, 30, 25, 20};
    std::vector<int> actualPostOrder = ledger->postOrderTreeTraversal(ledger->getBidTree());

    EXPECT_EQ(expectedPostOrder, actualPostOrder);
}

TEST_F(LedgerTests, TestAVLTreeRLRotateOnDelete){
    ledger->addLimitTicket(5, true, 80, 20);
    ledger->addLimitTicket(6, true, 80, 15);
    ledger->addLimitTicket(7, true, 80, 25);
    ledger->addLimitTicket(8, true, 80, 10);
    ledger->addLimitTicket(9, true, 80, 17);
    ledger->addLimitTicket(10, true, 80, 24);
    ledger->addLimitTicket(11, true, 80, 30);
    ledger->addLimitTicket(12, true, 80, 5);
    ledger->addLimitTicket(13, true, 80, 23);
    ledger->addLimitTicket(14, true, 80, 28);
    ledger->addLimitTicket(15, true, 80, 35);
    ledger->addLimitTicket(16, true, 80, 26);
    ledger->addLimitTicket(17, true, 80, 29);

    ledger->cancelLimitTicket(13);

    std::vector<int> expectedInOrder = {5, 10, 15, 17, 20, 24, 25, 26, 28, 29, 30, 35};
    std::vector<int> actualInOrder = ledger->inOrderTreeTraversal(ledger->getBidTree());

    EXPECT_EQ(expectedInOrder, actualInOrder);

    std::vector<int> expectedPreOrder = {20, 15, 10, 5, 17, 28, 25, 24, 26, 30, 29, 35};
    std::vector<int> actualPreOrder = ledger->preOrderTreeTraversal(ledger->getBidTree());

    EXPECT_EQ(expectedPreOrder, actualPreOrder);

    std::vector<int> expectedPostOrder = {5, 10, 17, 15, 24, 26, 25, 29, 35, 30, 28, 20};
    std::vector<int> actualPostOrder = ledger->postOrderTreeTraversal(ledger->getBidTree());

    EXPECT_EQ(expectedPostOrder, actualPostOrder);
}

TEST_F(LedgerTests, TestAVLTreeLRRotateOnDelete){
    ledger->addLimitTicket(5, true, 80, 20);
    ledger->addLimitTicket(6, true, 80, 15);
    ledger->addLimitTicket(7, true, 80, 25);
    ledger->addLimitTicket(8, true, 80, 10);
    ledger->addLimitTicket(9, true, 80, 17);
    ledger->addLimitTicket(10, true, 80, 24);
    ledger->addLimitTicket(11, true, 80, 30);
    ledger->addLimitTicket(12, true, 80, 5);
    ledger->addLimitTicket(13, true, 80, 13);
    ledger->addLimitTicket(14, true, 80, 19);
    ledger->addLimitTicket(15, true, 80, 35);
    ledger->addLimitTicket(16, true, 80, 12);
    ledger->addLimitTicket(17, true, 80, 14);

    ledger->cancelLimitTicket(14);

    std::vector<int> expectedInOrder = {5, 10, 12, 13, 14, 15, 17, 20, 24, 25, 30, 35};
    std::vector<int> actualInOrder = ledger->inOrderTreeTraversal(ledger->getBidTree());

    EXPECT_EQ(expectedInOrder, actualInOrder);

    std::vector<int> expectedPreOrder = {20, 13, 10, 5, 12, 15, 14, 17, 25, 24, 30, 35};
    std::vector<int> actualPreOrder = ledger->preOrderTreeTraversal(ledger->getBidTree());

    EXPECT_EQ(expectedPreOrder, actualPreOrder);

    std::vector<int> expectedPostOrder = {5, 12, 10, 14, 17, 15, 13, 24, 35, 30, 25, 20};
    std::vector<int> actualPostOrder = ledger->postOrderTreeTraversal(ledger->getBidTree());

    EXPECT_EQ(expectedPostOrder, actualPostOrder);
}

TEST_F(LedgerTests, TestAVLTreeRRRotateRootOnDelete){
    ledger->addLimitTicket(111, false, 43, 80);
    ledger->addLimitTicket(112, false, 43, 79);
    ledger->addLimitTicket(113, false, 543, 82);
    ledger->addLimitTicket(114, false, 46, 81);
    ledger->addLimitTicket(115, false, 46, 83);

    ledger->cancelLimitTicket(112);

    std::vector<int> expectedInOrder = {80, 81, 82, 83};
    std::vector<int> actualInOrder = ledger->inOrderTreeTraversal(ledger->getAskTree());

    EXPECT_EQ(expectedInOrder, actualInOrder);

    std::vector<int> expectedPreOrder = {82, 80, 81, 83};
    std::vector<int> actualPreOrder = ledger->preOrderTreeTraversal(ledger->getAskTree());

    EXPECT_EQ(expectedPreOrder, actualPreOrder);

    std::vector<int> expectedPostOrder = {81, 80, 83, 82};
    std::vector<int> actualPostOrder = ledger->postOrderTreeTraversal(ledger->getAskTree());

    EXPECT_EQ(expectedPostOrder, actualPostOrder);

    EXPECT_EQ(ledger->getAskTree()->getPrice(), 82);
    EXPECT_EQ(ledger->getAskTree()->getParent(), nullptr);
}

TEST_F(LedgerTests, TestAVLTreeLLRotateRootOnDelete){
    ledger->addLimitTicket(111, true, 43, 80);
    ledger->addLimitTicket(112, true, 543, 78);
    ledger->addLimitTicket(113, true, 543, 81);
    ledger->addLimitTicket(114, true, 46, 77);
    ledger->addLimitTicket(115, true, 46, 79);

    ledger->cancelLimitTicket(113);

    std::vector<int> expectedInOrder = {77, 78, 79, 80};
    std::vector<int> actualInOrder = ledger->inOrderTreeTraversal(ledger->getBidTree());

    EXPECT_EQ(expectedInOrder, actualInOrder);

    std::vector<int> expectedPreOrder = {78, 77, 80, 79};
    std::vector<int> actualPreOrder = ledger->preOrderTreeTraversal(ledger->getBidTree());

    EXPECT_EQ(expectedPreOrder, actualPreOrder);

    std::vector<int> expectedPostOrder = {77, 79, 80, 78};
    std::vector<int> actualPostOrder = ledger->postOrderTreeTraversal(ledger->getBidTree());

    EXPECT_EQ(expectedPostOrder, actualPostOrder);

    EXPECT_EQ(ledger->getBidTree()->getPrice(), 78);
    EXPECT_EQ(ledger->getBidTree()->getParent(), nullptr);
}

TEST_F(LedgerTests, TestAVLTreeRLRotateRootOnDelete){
    ledger->addLimitTicket(111, false, 43, 80);
    ledger->addLimitTicket(112, false, 43, 75);
    ledger->addLimitTicket(113, false, 543, 81);
    ledger->addLimitTicket(114, false, 46, 82);

    ledger->cancelLimitTicket(112);

    std::vector<int> expectedInOrder = {80, 81, 82};
    std::vector<int> actualInOrder = ledger->inOrderTreeTraversal(ledger->getAskTree());

    EXPECT_EQ(expectedInOrder, actualInOrder);

    std::vector<int> expectedPreOrder = {81, 80, 82};
    std::vector<int> actualPreOrder = ledger->preOrderTreeTraversal(ledger->getAskTree());

    EXPECT_EQ(expectedPreOrder, actualPreOrder);

    std::vector<int> expectedPostOrder = {80, 82, 81};
    std::vector<int> actualPostOrder = ledger->postOrderTreeTraversal(ledger->getAskTree());

    EXPECT_EQ(expectedPostOrder, actualPostOrder);

    EXPECT_EQ(ledger->getAskTree()->getPrice(), 81);
    EXPECT_EQ(ledger->getAskTree()->getParent(), nullptr);
}

TEST_F(LedgerTests, TestAVLTreeLRRotateRootOnDelete){
    ledger->addLimitTicket(111, true, 43, 80);
    ledger->addLimitTicket(112, true, 46, 78);
    ledger->addLimitTicket(113, true, 46, 85);
    ledger->addLimitTicket(114, true, 543, 79);

    ledger->cancelLimitTicket(113);

    std::vector<int> expectedInOrder = {78, 79, 80};
    std::vector<int> actualInOrder = ledger->inOrderTreeTraversal(ledger->getBidTree());

    EXPECT_EQ(expectedInOrder, actualInOrder);

    std::vector<int> expectedPreOrder = {79, 78, 80};
    std::vector<int> actualPreOrder = ledger->preOrderTreeTraversal(ledger->getBidTree());

    EXPECT_EQ(expectedPreOrder, actualPreOrder);

    std::vector<int> expectedPostOrder = {78, 80, 79};
    std::vector<int> actualPostOrder = ledger->postOrderTreeTraversal(ledger->getBidTree());

    EXPECT_EQ(expectedPostOrder, actualPostOrder);

    EXPECT_EQ(ledger->getBidTree()->getPrice(), 79);
    EXPECT_EQ(ledger->getBidTree()->getParent(), nullptr);
}

// Ledger edge tests
TEST_F(LedgerTests, TestUpdateBookEdgeOnInsertLowestSell){
    ledger->addLimitTicket(111, false, 43, 80);
    ledger->addLimitTicket(112, false, 46, 78);

    EXPECT_EQ(ledger->getBestAsk()->getPrice(), 78);
    
    ledger->addLimitTicket(113, false, 46, 77);

    EXPECT_EQ(ledger->getBestAsk()->getPrice(), 77);

    ledger->addLimitTicket(114, false, 46, 85);

    EXPECT_EQ(ledger->getBestAsk()->getPrice(), 77);
}

TEST_F(LedgerTests, TestUpdateBookEdgeOnInsertHighestBuy){
    ledger->addLimitTicket(111, true, 43, 80);
    ledger->addLimitTicket(112, true, 46, 78);

    EXPECT_EQ(ledger->getBestBid()->getPrice(), 80);
    
    ledger->addLimitTicket(113, true, 46, 82);

    EXPECT_EQ(ledger->getBestBid()->getPrice(), 82);

    ledger->addLimitTicket(114, true, 46, 70);

    EXPECT_EQ(ledger->getBestBid()->getPrice(), 82);
}

TEST_F(LedgerTests, TestUpdateBookEdgeOnDeleteLowestSell){
    ledger->addLimitTicket(111, false, 43, 80);
    ledger->addLimitTicket(112, false, 46, 78);
    ledger->addLimitTicket(113, false, 46, 77);

    EXPECT_EQ(ledger->getBestAsk()->getPrice(), 77);
    
    ledger->cancelLimitTicket(113);

    EXPECT_EQ(ledger->getBestAsk()->getPrice(), 78);
}

TEST_F(LedgerTests, TestUpdateBookEdgeOnDeleteHighestBuy){
    ledger->addLimitTicket(111, true, 43, 80);
    ledger->addLimitTicket(112, true, 46, 78);
    ledger->addLimitTicket(113, true, 46, 82);

    EXPECT_EQ(ledger->getBestBid()->getPrice(), 82);
    
    ledger->cancelLimitTicket(113);

    EXPECT_EQ(ledger->getBestBid()->getPrice(), 80);
}

TEST_F(LedgerTests, TestUpdateBookEdgeOnDeleteHighestBuyEmptyTree){
    ledger->addLimitTicket(111, true, 43, 80);

    EXPECT_EQ(ledger->getBestBid()->getPrice(), 80);

    ledger->cancelLimitTicket(111);

    EXPECT_EQ(ledger->getBestBid(), nullptr);
}

TEST_F(LedgerTests, TestUpdateBookEdgeOnDeleteLowestSellEmptyTree){
    ledger->addLimitTicket(111, false, 43, 80);

    EXPECT_EQ(ledger->getBestAsk()->getPrice(), 80);

    ledger->cancelLimitTicket(111);

    EXPECT_EQ(ledger->getBestAsk(), nullptr);
}

TEST_F(LedgerTests, TestUpdateBookEdgeOnDeleteHighestBuyRootLimit){
    ledger->addLimitTicket(111, true, 43, 80);
    ledger->addLimitTicket(112, true, 43, 75);

    EXPECT_EQ(ledger->getBestBid()->getPrice(), 80);

    ledger->cancelLimitTicket(111);

    EXPECT_EQ(ledger->getBestBid()->getPrice(), 75);
}

TEST_F(LedgerTests, TestUpdateBookEdgeOnDeleteLowestSellRootLimit){
    ledger->addLimitTicket(111, false, 10, 80);
    ledger->addLimitTicket(112, false, 20, 85);

    EXPECT_EQ(ledger->getBestAsk()->getPrice(), 80);

    ledger->cancelLimitTicket(111);

    EXPECT_EQ(ledger->getBestAsk()->getPrice(), 85);
}

TEST_F(LedgerTests, TestUpdateBookEdgeOnDeleteHighestBuyNotParent){
    ledger->addLimitTicket(111, true, 43, 80);
    ledger->addLimitTicket(112, true, 43, 75);
    ledger->addLimitTicket(113, true, 43, 85);
    ledger->addLimitTicket(114, true, 43, 82);

    EXPECT_EQ(ledger->getBestBid()->getPrice(), 85);

    ledger->cancelLimitTicket(113);

    EXPECT_EQ(ledger->getBestBid()->getPrice(), 82);
}

TEST_F(LedgerTests, TestUpdateBookEdgeOnDeleteLowestSellNotParent){
    ledger->addLimitTicket(111, false, 43, 80);
    ledger->addLimitTicket(112, false, 43, 75);
    ledger->addLimitTicket(113, false, 43, 85);
    ledger->addLimitTicket(114, false, 43, 76);

    EXPECT_EQ(ledger->getBestAsk()->getPrice(), 75);

    ledger->cancelLimitTicket(112);

    EXPECT_EQ(ledger->getBestAsk()->getPrice(), 76);
}

// Market orders tests
TEST_F(LedgerTests, TestBuyMarketOrderFilledBySingleOrder){
    ledger->addLimitTicket(111, false, 100, 80);
    ledger->addLimitTicket(112, false, 30, 80);

    EXPECT_EQ(ledger->getBestAsk()->getFrontTicket()->getQuantity(), 100);

    ledger->marketTicket(113, true, 20);

    EXPECT_EQ(ledger->getBestAsk()->getFrontTicket()->getQuantity(), 80);
}

TEST_F(LedgerTests, TestSellMarketOrderFilledBySingleOrder){
    ledger->addLimitTicket(111, true, 100, 80);

    EXPECT_EQ(ledger->getBestBid()->getFrontTicket()->getQuantity(), 100);

    ledger->marketTicket(112, false, 98);

    EXPECT_EQ(ledger->getBestBid()->getFrontTicket()->getQuantity(), 2);
}

TEST_F(LedgerTests, TestBuyMarketOrderFilledByMultipleOrders){
    ledger->addLimitTicket(111, false, 10, 80);
    ledger->addLimitTicket(112, false, 10, 80);
    ledger->addLimitTicket(113, false, 10, 80);
    ledger->addLimitTicket(114, false, 30, 80);

    EXPECT_EQ(ledger->getBestAsk()->getFrontTicket()->getQuantity(), 10);
    EXPECT_EQ(ledger->getBestAsk()->getAggregateQty(), 60);

    ledger->marketTicket(115, true, 40);

    EXPECT_EQ(ledger->getBestAsk()->getFrontTicket()->getQuantity(), 20);
    EXPECT_EQ(ledger->getBestAsk()->getAggregateQty(), 20);
}

TEST_F(LedgerTests, TestSellMarketOrderFilledByMultipleOrders){
    ledger->addLimitTicket(111, true, 5, 80);
    ledger->addLimitTicket(112, true, 7, 80);
    ledger->addLimitTicket(113, true, 31, 80);
    ledger->addLimitTicket(114, true, 9, 80);

    EXPECT_EQ(ledger->getBestBid()->getFrontTicket()->getQuantity(), 5);
    EXPECT_EQ(ledger->getBestBid()->getAggregateQty(), 52);

    ledger->marketTicket(115, false, 12);

    EXPECT_EQ(ledger->getBestBid()->getFrontTicket()->getQuantity(), 31);
    EXPECT_EQ(ledger->getBestBid()->getAggregateQty(), 40);
}

TEST_F(LedgerTests, TestBuyMarketOrderPerfectlyFilledBySingleOrder){
    ledger->addLimitTicket(111, false, 15, 80);
    ledger->addLimitTicket(112, false, 21, 80);

    EXPECT_EQ(ledger->getBestAsk()->getFrontTicket()->getQuantity(), 15);

    ledger->marketTicket(115, true, 15);

    EXPECT_EQ(ledger->getBestAsk()->getFrontTicket()->getQuantity(), 21);
}

TEST_F(LedgerTests, TestSellMarketOrderPerfectlyFilledBySingleOrder){
    ledger->addLimitTicket(111, true, 1153, 80);
    ledger->addLimitTicket(112, true, 832, 80);

    EXPECT_EQ(ledger->getBestBid()->getFrontTicket()->getQuantity(), 1153);

    ledger->marketTicket(115, false, 1153);

    EXPECT_EQ(ledger->getBestBid()->getFrontTicket()->getQuantity(), 832);
}

TEST_F(LedgerTests, TestBuyMarketOrderGoingIntoDifferentLimit){
    ledger->addLimitTicket(111, false, 10, 80);
    ledger->addLimitTicket(112, false, 5, 80);
    ledger->addLimitTicket(113, false, 20, 85);

    EXPECT_EQ(ledger->getBestAsk()->getFrontTicket()->getQuantity(), 10);

    ledger->marketTicket(115, true, 20);

    EXPECT_EQ(ledger->getBestAsk()->getFrontTicket()->getQuantity(), 15);
    EXPECT_EQ(ledger->getBestAsk()->getPrice(), 85);
}

TEST_F(LedgerTests, TestSellMarketOrderGoingIntoDifferentLimit){
    ledger->addLimitTicket(111, true, 10, 80);
    ledger->addLimitTicket(112, true, 20, 80);
    ledger->addLimitTicket(113, true, 7, 85);

    EXPECT_EQ(ledger->getBestBid()->getFrontTicket()->getQuantity(), 7);

    ledger->marketTicket(115, false, 18);

    EXPECT_EQ(ledger->getBestBid()->getFrontTicket()->getQuantity(), 19);
    EXPECT_EQ(ledger->getBestBid()->getPrice(), 80);
}

TEST_F(LedgerTests, TestBuyMarketOrderEmptySellTree){
    EXPECT_EQ(ledger->getBestAsk(), nullptr);

    ledger->marketTicket(115, true, 18);

    EXPECT_EQ(ledger->getBestAsk(), nullptr);
}

TEST_F(LedgerTests, TestSellMarketOrderEmptyBuyTree){
    ledger->addLimitTicket(111, true, 10, 80);
    ledger->addLimitTicket(113, true, 7, 85);

    EXPECT_EQ(ledger->getBestBid()->getFrontTicket()->getQuantity(), 7);

    ledger->marketTicket(115, false, 18);

    EXPECT_EQ(ledger->getBestBid(), nullptr);
}

// Modifying orders tests
TEST_F(LedgerTests, TestModifyOrderToExistingLimit){
    ledger->addLimitTicket(111, true, 10, 80);
    ledger->addLimitTicket(112, true, 20, 80);
    ledger->addLimitTicket(113, true, 7, 85);
    ledger->addLimitTicket(114, true, 14, 85);

    ledger->modifyLimitTicket(113, 40, 80);

    PriceLevel* limit1 = ledger->searchLevelMaps(80, true);
    PriceLevel* limit2 = ledger->searchLevelMaps(85, true);

    EXPECT_EQ(limit1->getFrontTicket()->getTicketId(), 111);
    EXPECT_EQ(limit2->getFrontTicket()->getTicketId(), 114);
    EXPECT_EQ(limit1->getAggregateQty(), 70);
    EXPECT_EQ(limit2->getAggregateQty(), 14);
}

TEST_F(LedgerTests, TestModifyOrderToNewLimit){
    ledger->addLimitTicket(111, true, 10, 80);
    ledger->addLimitTicket(112, true, 20, 80);
    ledger->addLimitTicket(113, true, 7, 85);
    ledger->addLimitTicket(114, true, 14, 85);

    ledger->modifyLimitTicket(113, 40, 82);

    PriceLevel* limit1 = ledger->searchLevelMaps(82, true);
    PriceLevel* limit2 = ledger->searchLevelMaps(85, true);

    EXPECT_EQ(limit1->getFrontTicket()->getTicketId(), 113);
    EXPECT_EQ(limit2->getFrontTicket()->getTicketId(), 114);
    EXPECT_EQ(limit1->getAggregateQty(), 40);
    EXPECT_EQ(limit2->getAggregateQty(), 14);
}

TEST_F(LedgerTests, TestModifyOrderInvalidOrderId){
    ledger->addLimitTicket(111, true, 10, 80);
    ledger->addLimitTicket(112, true, 20, 80);
    ledger->addLimitTicket(113, true, 7, 85);
    ledger->addLimitTicket(114, true, 14, 85);

    ledger->modifyLimitTicket(110, 40, 82);

    PriceLevel* limit1 = ledger->searchLevelMaps(80, true);
    PriceLevel* limit2 = ledger->searchLevelMaps(85, true);

    EXPECT_EQ(limit1->getFrontTicket()->getTicketId(), 111);
    EXPECT_EQ(limit2->getFrontTicket()->getTicketId(), 113);
    EXPECT_EQ(limit1->getAggregateQty(), 30);
    EXPECT_EQ(limit2->getAggregateQty(), 21);
    EXPECT_EQ(ledger->searchLevelMaps(82, true), nullptr);
    EXPECT_EQ(ledger->searchTicketRegistry(110), nullptr);
}

// PriceLevel order that is a market order tests
TEST_F(LedgerTests, TestAddingSellLimitOrderWhichIsAMarketOrder) {
    ledger->addLimitTicket(357, true, 40, 100);
    ledger->addLimitTicket(222, false, 35, 100);

    EXPECT_EQ(ledger->getBestBid()->getAggregateQty(), 5);
    EXPECT_EQ(ledger->getBestBid()->getFrontTicket()->getQuantity(), 5);
    EXPECT_EQ(ledger->getBestAsk(), nullptr);
}

TEST_F(LedgerTests, TestAddingSellLimitOrderWhichIsAMarketOrderAcrossMultipleLimits) {
    ledger->addLimitTicket(357, true, 15, 110);
    ledger->addLimitTicket(358, true, 35, 100);
    ledger->addLimitTicket(359, true, 35, 100);
    ledger->addLimitTicket(222, false, 40, 100);

    EXPECT_EQ(ledger->getBestBid()->getPrice(), 100);
    EXPECT_EQ(ledger->getBestBid()->getAggregateQty(), 45);
    EXPECT_EQ(ledger->getBestBid()->getFrontTicket()->getQuantity(), 10);
    EXPECT_EQ(ledger->getBestAsk(), nullptr);
}

TEST_F(LedgerTests, TestAddingSellLimitOrderWhichIsOnlyPartiallyAMarketOrder) {
    ledger->addLimitTicket(357, true, 40, 100);
    ledger->addLimitTicket(357, true, 40, 99);
    ledger->addLimitTicket(222, false, 45, 100);

    EXPECT_EQ(ledger->getBestBid()->getAggregateQty(), 40);
    EXPECT_EQ(ledger->getBestBid()->getFrontTicket()->getQuantity(), 40);
    EXPECT_EQ(ledger->getBestAsk()->getAggregateQty(), 5);
    EXPECT_EQ(ledger->getBestAsk()->getFrontTicket()->getQuantity(), 5);
}

TEST_F(LedgerTests, TestAddingBuyLimitOrderWhichIsAMarketOrder) {
    ledger->addLimitTicket(357, false, 40, 100);
    ledger->addLimitTicket(222, true, 35, 100);

    EXPECT_EQ(ledger->getBestAsk()->getAggregateQty(), 5);
    EXPECT_EQ(ledger->getBestAsk()->getFrontTicket()->getQuantity(), 5);
    EXPECT_EQ(ledger->getBestBid(), nullptr);
}

TEST_F(LedgerTests, TestAddingBuyLimitOrderWhichIsAMarketOrderAcrossMultipleLimits) {
    ledger->addLimitTicket(357, false, 15, 90);
    ledger->addLimitTicket(358, false, 35, 100);
    ledger->addLimitTicket(359, false, 35, 100);
    ledger->addLimitTicket(222, true, 40, 100);

    EXPECT_EQ(ledger->getBestAsk()->getPrice(), 100);
    EXPECT_EQ(ledger->getBestAsk()->getAggregateQty(), 45);
    EXPECT_EQ(ledger->getBestAsk()->getFrontTicket()->getQuantity(), 10);
    EXPECT_EQ(ledger->getBestBid(), nullptr);
}

TEST_F(LedgerTests, TestAddingBuyLimitOrderWhichIsOnlyPartiallyAMarketOrder) {
    ledger->addLimitTicket(357, false, 40, 100);
    ledger->addLimitTicket(357, false, 40, 101);
    ledger->addLimitTicket(222, true, 45, 100);

    EXPECT_EQ(ledger->getBestAsk()->getAggregateQty(), 40);
    EXPECT_EQ(ledger->getBestAsk()->getFrontTicket()->getQuantity(), 40);
    EXPECT_EQ(ledger->getBestBid()->getAggregateQty(), 5);
    EXPECT_EQ(ledger->getBestBid()->getFrontTicket()->getQuantity(), 5);
}

// Adding stop orders tests
TEST_F(LedgerTests, TestAddingAStopOrder) {
    EXPECT_EQ(ledger->searchTicketRegistry(357), nullptr);
    EXPECT_EQ(ledger->searchStopLevels(100), nullptr);

    ledger->addStopTicket(357, true, 27, 100);

    EXPECT_EQ(ledger->searchTicketRegistry(357)->getQuantity(), 27);
    EXPECT_EQ(ledger->searchStopLevels(100)->getAggregateQty(), 27);
    EXPECT_EQ(ledger->searchStopLevels(20), nullptr);

    ledger->addStopTicket(222, false, 35, 110);

    EXPECT_EQ(ledger->searchStopLevels(110)->getAggregateQty(), 35);
}

// Cancelling stop orders tests
TEST_F(LedgerTests, TestCancelStopOrderLeavingNonEmptyLimit){
    ledger->addStopTicket(5, true, 80, 20);
    ledger->addStopTicket(6, true, 32, 20);
    ledger->addStopTicket(7, true, 111, 20);

    EXPECT_EQ(ledger->searchStopLevels(20)->getTicketCount(), 3);
    EXPECT_EQ(ledger->searchStopLevels(20)->getAggregateQty(), 223);

    ledger->cancelStopTicket(6);

    EXPECT_EQ(ledger->searchStopLevels(20)->getTicketCount(), 2);
    EXPECT_EQ(ledger->searchStopLevels(20)->getAggregateQty(), 191);

    ledger->cancelStopTicket(7);

    EXPECT_EQ(ledger->searchStopLevels(20)->getTicketCount(), 1);
    EXPECT_EQ(ledger->searchStopLevels(20)->getAggregateQty(), 80);
}

TEST_F(LedgerTests, TestStopLevelHeadOrderChangeOnStopOrderCancel){
    ledger->addStopTicket(5, true, 80, 20);
    ledger->addStopTicket(6, true, 32, 20);
    ledger->addStopTicket(7, true, 111, 20);

    PriceLevel* stop = ledger->searchStopLevels(20);

    EXPECT_EQ(stop->getFrontTicket()->getTicketId(), 5);

    ledger->cancelStopTicket(5);
    
    EXPECT_EQ(stop->getFrontTicket()->getTicketId(), 6);
}

TEST_F(LedgerTests, TestStopLevelHeadOrderChangeOnStopOrderCancelLeavingEmptyLimit){
    ledger->addStopTicket(5, true, 80, 20);

    PriceLevel* stop = ledger->searchStopLevels(20);

    EXPECT_EQ(stop->getFrontTicket()->getTicketId(), 5);

    ledger->cancelStopTicket(5);
    
    EXPECT_EQ(stop->getFrontTicket(), nullptr);
}

TEST_F(LedgerTests, TestCancelStopOrderLeavingEmptyLimit){
    ledger->addStopTicket(5, true, 80, 20);
    ledger->addStopTicket(6, true, 80, 15);
    PriceLevel* stop1 = ledger->searchStopLevels(20);
    PriceLevel* stop2 = ledger->searchStopLevels(15);

    EXPECT_EQ(stop2->getFrontTicket()->getTicketId(), 6);
    EXPECT_EQ(stop1->getLeftChild()->getPrice(), 15);

    ledger->cancelStopTicket(6);
    
    EXPECT_EQ(ledger->searchLevelMaps(15, true), nullptr);
    EXPECT_EQ(stop1->getLeftChild(), nullptr);
}

// Modifying stop orders tests
TEST_F(LedgerTests, TestModifyStopOrderToExistingStopLevel){
    ledger->addStopTicket(111, true, 10, 80);
    ledger->addStopTicket(112, true, 20, 80);
    ledger->addStopTicket(113, true, 7, 85);
    ledger->addStopTicket(114, true, 14, 85);

    ledger->modifyStopTicket(113, 40, 80);

    PriceLevel* stop1 = ledger->searchStopLevels(80);
    PriceLevel* stop2 = ledger->searchStopLevels(85);

    EXPECT_EQ(stop1->getFrontTicket()->getTicketId(), 111);
    EXPECT_EQ(stop2->getFrontTicket()->getTicketId(), 114);
    EXPECT_EQ(stop1->getAggregateQty(), 70);
    EXPECT_EQ(stop2->getAggregateQty(), 14);
}

TEST_F(LedgerTests, TestModifyStopOrderToNewStopLevel){
    ledger->addStopTicket(111, true, 10, 80);
    ledger->addStopTicket(112, true, 20, 80);
    ledger->addStopTicket(113, true, 7, 85);
    ledger->addStopTicket(114, true, 14, 85);

    ledger->modifyStopTicket(113, 40, 82);

    PriceLevel* stop1 = ledger->searchStopLevels(82);
    PriceLevel* stop2 = ledger->searchStopLevels(85);

    EXPECT_EQ(stop1->getFrontTicket()->getTicketId(), 113);
    EXPECT_EQ(stop2->getFrontTicket()->getTicketId(), 114);
    EXPECT_EQ(stop1->getAggregateQty(), 40);
    EXPECT_EQ(stop2->getAggregateQty(), 14);
}

TEST_F(LedgerTests, TestModifyStopOrderInvalidOrderId){
    ledger->addStopTicket(111, true, 10, 80);
    ledger->addStopTicket(112, true, 20, 80);
    ledger->addStopTicket(113, true, 7, 85);
    ledger->addStopTicket(114, true, 14, 85);

    ledger->modifyStopTicket(110, 40, 82);

    PriceLevel* stop1 = ledger->searchStopLevels(80);
    PriceLevel* stop2 = ledger->searchStopLevels(85);

    EXPECT_EQ(stop1->getFrontTicket()->getTicketId(), 111);
    EXPECT_EQ(stop2->getFrontTicket()->getTicketId(), 113);
    EXPECT_EQ(stop1->getAggregateQty(), 30);
    EXPECT_EQ(stop2->getAggregateQty(), 21);
    EXPECT_EQ(ledger->searchStopLevels(82), nullptr);
    EXPECT_EQ(ledger->searchTicketRegistry(110), nullptr);
}

// Stop ledger edge tests
TEST_F(LedgerTests, TestUpdateStopBookEdgeOnInsertHighestStopSell){
    ledger->addStopTicket(111, false, 43, 80);
    ledger->addStopTicket(112, false, 46, 78);

    EXPECT_EQ(ledger->getHighestStopAsk()->getPrice(), 80);
    
    ledger->addStopTicket(113, false, 46, 81);

    EXPECT_EQ(ledger->getHighestStopAsk()->getPrice(), 81);

    ledger->addStopTicket(114, false, 46, 79);

    EXPECT_EQ(ledger->getHighestStopAsk()->getPrice(), 81);
}

TEST_F(LedgerTests, TestUpdateStopBookEdgeOnInsertLowestStopBuy){
    ledger->addStopTicket(111, true, 43, 80);
    ledger->addStopTicket(112, true, 46, 78);

    EXPECT_EQ(ledger->getLowestStopBid()->getPrice(), 78);
    
    ledger->addStopTicket(113, true, 46, 82);

    EXPECT_EQ(ledger->getLowestStopBid()->getPrice(), 78);

    ledger->addStopTicket(114, true, 46, 70);

    EXPECT_EQ(ledger->getLowestStopBid()->getPrice(), 70);
}

TEST_F(LedgerTests, TestUpdateStopBookEdgeOnDeleteHighestStopSell){
    ledger->addStopTicket(111, false, 43, 78);
    ledger->addStopTicket(112, false, 46, 80);
    ledger->addStopTicket(113, false, 46, 77);

    EXPECT_EQ(ledger->getHighestStopAsk()->getPrice(), 80);
    
    ledger->cancelStopTicket(112);

    EXPECT_EQ(ledger->getHighestStopAsk()->getPrice(), 78);
}

TEST_F(LedgerTests, TestUpdateStopBookEdgeOnDeleteLowestStopBuy){
    ledger->addStopTicket(111, true, 43, 80);
    ledger->addStopTicket(112, true, 46, 78);
    ledger->addStopTicket(113, true, 46, 82);

    EXPECT_EQ(ledger->getLowestStopBid()->getPrice(), 78);
    
    ledger->cancelStopTicket(112);

    EXPECT_EQ(ledger->getLowestStopBid()->getPrice(), 80);
}

TEST_F(LedgerTests, TestUpdateStopBookEdgeOnDeleteLowestStopBuyEmptyTree){
    ledger->addStopTicket(111, true, 43, 80);

    EXPECT_EQ(ledger->getLowestStopBid()->getPrice(), 80);

    ledger->cancelStopTicket(111);

    EXPECT_EQ(ledger->getLowestStopBid(), nullptr);
}

TEST_F(LedgerTests, TestUpdateStopBookEdgeOnDeleteHighestStopSellEmptyTree){
    ledger->addStopTicket(111, false, 43, 80);

    EXPECT_EQ(ledger->getHighestStopAsk()->getPrice(), 80);

    ledger->cancelStopTicket(111);

    EXPECT_EQ(ledger->getHighestStopAsk(), nullptr);
}

TEST_F(LedgerTests, TestUpdateStopBookEdgeOnDeleteLowestStopBuyRootLimit){
    ledger->addStopTicket(111, true, 43, 75);
    ledger->addStopTicket(112, true, 43, 80);

    EXPECT_EQ(ledger->getLowestStopBid()->getPrice(), 75);

    ledger->cancelStopTicket(111);

    EXPECT_EQ(ledger->getLowestStopBid()->getPrice(), 80);
}

TEST_F(LedgerTests, TestUpdateBookEdgeOnDeleteHighestStopSellRootLimit){
    ledger->addStopTicket(111, false, 10, 85);
    ledger->addStopTicket(112, false, 20, 80);

    EXPECT_EQ(ledger->getHighestStopAsk()->getPrice(), 85);

    ledger->cancelStopTicket(111);

    EXPECT_EQ(ledger->getHighestStopAsk()->getPrice(), 80);
}

TEST_F(LedgerTests, TestUpdateStopBookEdgeOnDeleteLowestStopBuyNotParent){
    ledger->addStopTicket(111, true, 43, 80);
    ledger->addStopTicket(112, true, 43, 75);
    ledger->addStopTicket(113, true, 43, 85);
    ledger->addStopTicket(114, true, 43, 76);

    EXPECT_EQ(ledger->getLowestStopBid()->getPrice(), 75);

    ledger->cancelStopTicket(112);

    EXPECT_EQ(ledger->getLowestStopBid()->getPrice(), 76);
}

TEST_F(LedgerTests, TestUpdateStopBookEdgeOnDeleteHighestStopSellNotParent){
    ledger->addStopTicket(111, false, 43, 80);
    ledger->addStopTicket(112, false, 43, 75);
    ledger->addStopTicket(113, false, 43, 85);
    ledger->addStopTicket(114, false, 43, 82);

    EXPECT_EQ(ledger->getHighestStopAsk()->getPrice(), 85);

    ledger->cancelStopTicket(113);

    EXPECT_EQ(ledger->getHighestStopAsk()->getPrice(), 82);
}

// Stop orders being triggered tests
TEST_F(LedgerTests, TestStopOrdersTriggeredByMarketSellOrder){
    ledger->addLimitTicket(111, true, 10, 100);
    ledger->addLimitTicket(112, true, 10, 99);
    ledger->addLimitTicket(113, true, 10, 98);

    ledger->addStopTicket(114, false, 15, 99);

    EXPECT_EQ(ledger->getBestBid()->getPrice(), 100);
    EXPECT_EQ(ledger->getHighestStopAsk()->getPrice(), 99);

    ledger->marketTicket(115, false, 11);

    EXPECT_EQ(ledger->getBestBid()->getPrice(), 98);
    EXPECT_EQ(ledger->getBestBid()->getAggregateQty(), 4);
    EXPECT_EQ(ledger->getHighestStopAsk(), nullptr);
}

TEST_F(LedgerTests, TestStopOrdersTriggeredByMarketBuyOrder){
    ledger->addLimitTicket(111, false, 10, 100);
    ledger->addLimitTicket(112, false, 10, 99);
    ledger->addLimitTicket(113, false, 10, 98);

    ledger->addStopTicket(114, true, 15, 99);

    EXPECT_EQ(ledger->getBestAsk()->getPrice(), 98);
    EXPECT_EQ(ledger->getLowestStopBid()->getPrice(), 99);

    ledger->marketTicket(115, true, 11);

    EXPECT_EQ(ledger->getBestAsk()->getPrice(), 100);
    EXPECT_EQ(ledger->getBestAsk()->getAggregateQty(), 4);
    EXPECT_EQ(ledger->getLowestStopBid(), nullptr);
}

TEST_F(LedgerTests, TestMultipleStopOrderLevelsTriggeredByMarketSellOrder){
    ledger->addLimitTicket(111, true, 10, 100);
    ledger->addLimitTicket(112, true, 10, 98);
    ledger->addLimitTicket(113, true, 30, 97);

    ledger->addStopTicket(114, false, 15, 99);
    ledger->addStopTicket(115, false, 15, 98);
    ledger->addStopTicket(116, false, 15, 96);

    std::vector<int> expectedPreOrder = {98, 96, 99};
    std::vector<int> actualPreOrder = ledger->preOrderTreeTraversal(ledger->getStopAskTree());
    EXPECT_EQ(expectedPreOrder, actualPreOrder);

    EXPECT_EQ(ledger->getBestBid()->getPrice(), 100);
    EXPECT_EQ(ledger->getHighestStopAsk()->getPrice(), 99);

    ledger->marketTicket(117, false, 11);

    EXPECT_EQ(ledger->getBestBid()->getPrice(), 97);
    EXPECT_EQ(ledger->getBestBid()->getAggregateQty(), 9);
    EXPECT_EQ(ledger->getHighestStopAsk()->getPrice(), 96);
}

TEST_F(LedgerTests, TestMultipleStopOrderLevelsTriggeredByMarketBuyOrder){
    ledger->addLimitTicket(111, false, 30, 101);
    ledger->addLimitTicket(112, false, 10, 100);
    ledger->addLimitTicket(113, false, 10, 98);

    ledger->addStopTicket(114, true, 15, 99);
    ledger->addStopTicket(115, true, 15, 100);
    ledger->addStopTicket(116, true, 15, 102);

    std::vector<int> expectedPreOrder = {100, 99, 102};
    std::vector<int> actualPreOrder = ledger->preOrderTreeTraversal(ledger->getStopBidTree());
    EXPECT_EQ(expectedPreOrder, actualPreOrder);

    EXPECT_EQ(ledger->getBestAsk()->getPrice(), 98);
    EXPECT_EQ(ledger->getLowestStopBid()->getPrice(), 99);

    ledger->marketTicket(117, true, 11);

    EXPECT_EQ(ledger->getBestAsk()->getPrice(), 101);
    EXPECT_EQ(ledger->getBestAsk()->getAggregateQty(), 9);
    EXPECT_EQ(ledger->getLowestStopBid()->getPrice(), 102);
}

TEST_F(LedgerTests, TestStopSellOrderTriggeringFurtherStopSellOrder){
    ledger->addLimitTicket(111, true, 10, 100);
    ledger->addLimitTicket(112, true, 10, 99);
    ledger->addLimitTicket(113, true, 30, 97);

    ledger->addStopTicket(114, false, 15, 99);
    ledger->addStopTicket(115, false, 15, 98);
    ledger->addStopTicket(116, false, 15, 96);

    EXPECT_EQ(ledger->getBestBid()->getPrice(), 100);
    EXPECT_EQ(ledger->getHighestStopAsk()->getPrice(), 99);

    ledger->marketTicket(117, false, 11);

    EXPECT_EQ(ledger->getBestBid()->getPrice(), 97);
    EXPECT_EQ(ledger->getBestBid()->getAggregateQty(), 9);
    EXPECT_EQ(ledger->getHighestStopAsk()->getPrice(), 96);
}

TEST_F(LedgerTests, TestStopBuyOrderTriggeringFurtherStopBuyOrder){
    ledger->addLimitTicket(111, false, 30, 101);
    ledger->addLimitTicket(112, false, 10, 99);
    ledger->addLimitTicket(113, false, 10, 98);

    ledger->addStopTicket(114, true, 15, 99);
    ledger->addStopTicket(115, true, 15, 100);
    ledger->addStopTicket(116, true, 15, 102);

    EXPECT_EQ(ledger->getBestAsk()->getPrice(), 98);
    EXPECT_EQ(ledger->getLowestStopBid()->getPrice(), 99);

    ledger->marketTicket(117, true, 11);

    EXPECT_EQ(ledger->getBestAsk()->getPrice(), 101);
    EXPECT_EQ(ledger->getBestAsk()->getAggregateQty(), 9);
    EXPECT_EQ(ledger->getLowestStopBid()->getPrice(), 102);
}

TEST_F(LedgerTests, TestStopOrderTriggeringTwoFurtherIterationsOfStopOrders){
    ledger->addLimitTicket(111, true, 10, 100);
    ledger->addLimitTicket(112, true, 10, 99);
    ledger->addLimitTicket(113, true, 20, 97);
    ledger->addLimitTicket(118, true, 30, 95);

    ledger->addStopTicket(114, false, 15, 99);
    ledger->addStopTicket(115, false, 15, 98);
    ledger->addStopTicket(116, false, 15, 96);
    ledger->addStopTicket(119, false, 15, 94);

    EXPECT_EQ(ledger->getBestBid()->getPrice(), 100);
    EXPECT_EQ(ledger->getHighestStopAsk()->getPrice(), 99);

    ledger->marketTicket(117, false, 11);

    EXPECT_EQ(ledger->getBestBid()->getPrice(), 95);
    EXPECT_EQ(ledger->getBestBid()->getAggregateQty(), 14);
    EXPECT_EQ(ledger->getHighestStopAsk()->getPrice(), 94);
}

TEST_F(LedgerTests, TestStopOrdersTriggeredBySellLimitOrderWhichIsAMarketOrder){
    ledger->addLimitTicket(111, true, 10, 100);
    ledger->addLimitTicket(112, true, 10, 99);
    ledger->addLimitTicket(113, true, 10, 98);

    ledger->addStopTicket(114, false, 15, 99);

    EXPECT_EQ(ledger->getBestBid()->getPrice(), 100);
    EXPECT_EQ(ledger->getHighestStopAsk()->getPrice(), 99);

    ledger->addLimitTicket(115, false, 11, 99);

    EXPECT_EQ(ledger->getBestBid()->getPrice(), 98);
    EXPECT_EQ(ledger->getBestBid()->getAggregateQty(), 4);
    EXPECT_EQ(ledger->getHighestStopAsk(), nullptr);
}

TEST_F(LedgerTests, TestStopOrdersTriggeredByBuyLimitOrderWhichIsAMarketOrder){
    ledger->addLimitTicket(111, false, 10, 100);
    ledger->addLimitTicket(112, false, 10, 99);
    ledger->addLimitTicket(113, false, 10, 98);

    ledger->addStopTicket(114, true, 15, 99);

    EXPECT_EQ(ledger->getBestAsk()->getPrice(), 98);
    EXPECT_EQ(ledger->getLowestStopBid()->getPrice(), 99);

    ledger->addLimitTicket(115, true, 11, 99);

    EXPECT_EQ(ledger->getBestAsk()->getPrice(), 100);
    EXPECT_EQ(ledger->getBestAsk()->getAggregateQty(), 4);
    EXPECT_EQ(ledger->getLowestStopBid(), nullptr);
}

TEST_F(LedgerTests, TestBuyStopOrderWhenEmptySellTree){
    ledger->addLimitTicket(111, true, 10, 100);
    ledger->addLimitTicket(112, true, 10, 99);

    ledger->addStopTicket(114, false, 15, 99);

    EXPECT_EQ(ledger->getBestBid()->getPrice(), 100);
    EXPECT_EQ(ledger->getHighestStopAsk()->getPrice(), 99);

    ledger->marketTicket(115, false, 11);

    EXPECT_EQ(ledger->getBestBid(), nullptr);
    EXPECT_EQ(ledger->getHighestStopAsk(), nullptr);
}

TEST_F(LedgerTests, TestSellStopOrderWhenEmptyBuyTree){
    ledger->addLimitTicket(112, false, 10, 99);
    ledger->addLimitTicket(113, false, 10, 98);

    ledger->addStopTicket(114, true, 15, 99);

    EXPECT_EQ(ledger->getBestAsk()->getPrice(), 98);
    EXPECT_EQ(ledger->getLowestStopBid()->getPrice(), 99);

    ledger->marketTicket(115, true, 11);

    EXPECT_EQ(ledger->getBestAsk(), nullptr);
    EXPECT_EQ(ledger->getLowestStopBid(), nullptr);
}

// Stop order that is a market order tests
TEST_F(LedgerTests, TestAddingSellStopOrderWhichIsAMarketOrder) {
    ledger->addLimitTicket(357, true, 40, 100);
    ledger->addStopTicket(222, false, 35, 100);

    EXPECT_EQ(ledger->getBestBid()->getAggregateQty(), 5);
    EXPECT_EQ(ledger->getBestBid()->getFrontTicket()->getQuantity(), 5);
    EXPECT_EQ(ledger->getHighestStopAsk(), nullptr);
}

TEST_F(LedgerTests, TestAddingSellStopOrderWhichIsAMarketOrderAcrossMultipleLimits) {
    ledger->addLimitTicket(357, true, 15, 100);
    ledger->addLimitTicket(358, true, 35, 90);
    ledger->addLimitTicket(359, true, 35, 90);
    ledger->addStopTicket(222, false, 40, 100);

    EXPECT_EQ(ledger->getBestBid()->getPrice(), 90);
    EXPECT_EQ(ledger->getBestBid()->getAggregateQty(), 45);
    EXPECT_EQ(ledger->getBestBid()->getFrontTicket()->getQuantity(), 10);
    EXPECT_EQ(ledger->getHighestStopAsk(), nullptr);
}

TEST_F(LedgerTests, TestAddingBuyStopOrderWhichIsAMarketOrder) {
    ledger->addLimitTicket(357, false, 40, 100);
    ledger->addStopTicket(222, true, 35, 100);

    EXPECT_EQ(ledger->getBestAsk()->getAggregateQty(), 5);
    EXPECT_EQ(ledger->getBestAsk()->getFrontTicket()->getQuantity(), 5);
    EXPECT_EQ(ledger->getLowestStopBid(), nullptr);
}

TEST_F(LedgerTests, TestAddingBuyStopOrderWhichIsAMarketOrderAcrossMultipleLimits) {
    ledger->addLimitTicket(357, false, 15, 100);
    ledger->addLimitTicket(358, false, 35, 110);
    ledger->addLimitTicket(359, false, 35, 110);
    ledger->addStopTicket(222, true, 40, 100);

    EXPECT_EQ(ledger->getBestAsk()->getPrice(), 110);
    EXPECT_EQ(ledger->getBestAsk()->getAggregateQty(), 45);
    EXPECT_EQ(ledger->getBestAsk()->getFrontTicket()->getQuantity(), 10);
    EXPECT_EQ(ledger->getLowestStopBid(), nullptr);
}

// Adding stop limit orders tests
TEST_F(LedgerTests, TestAddingAStopLimitOrder) {
    EXPECT_EQ(ledger->searchTicketRegistry(357), nullptr);
    EXPECT_EQ(ledger->searchStopLevels(100), nullptr);

    ledger->addStopLimitTicket(357, true, 27, 110, 100);

    EXPECT_EQ(ledger->searchTicketRegistry(357)->getQuantity(), 27);
    EXPECT_EQ(ledger->searchStopLevels(100)->getAggregateQty(), 27);
    EXPECT_EQ(ledger->searchStopLevels(20), nullptr);

    ledger->addStopLimitTicket(222, false, 35, 105, 110);

    EXPECT_EQ(ledger->searchStopLevels(110)->getAggregateQty(), 35);
}

// Cancelling stop limit orders tests
TEST_F(LedgerTests, TestCancelStopLimitOrderLeavingNonEmptyLimit){
    ledger->addStopLimitTicket(5, true, 80, 25, 20);
    ledger->addStopLimitTicket(6, true, 32, 22, 20);
    ledger->addStopLimitTicket(7, true, 111, 23, 20);

    EXPECT_EQ(ledger->searchStopLevels(20)->getTicketCount(), 3);
    EXPECT_EQ(ledger->searchStopLevels(20)->getAggregateQty(), 223);

    ledger->cancelStopLimitTicket(6);

    EXPECT_EQ(ledger->searchStopLevels(20)->getTicketCount(), 2);
    EXPECT_EQ(ledger->searchStopLevels(20)->getAggregateQty(), 191);

    ledger->cancelStopLimitTicket(7);

    EXPECT_EQ(ledger->searchStopLevels(20)->getTicketCount(), 1);
    EXPECT_EQ(ledger->searchStopLevels(20)->getAggregateQty(), 80);
}

TEST_F(LedgerTests, TestStopLevelHeadOrderChangeOnStopLimitOrderCancel){
    ledger->addStopLimitTicket(5, true, 80, 25, 20);
    ledger->addStopLimitTicket(6, true, 32, 23, 20);
    ledger->addStopLimitTicket(7, true, 111, 25, 20);

    PriceLevel* stop = ledger->searchStopLevels(20);

    EXPECT_EQ(stop->getFrontTicket()->getTicketId(), 5);

    ledger->cancelStopLimitTicket(5);
    
    EXPECT_EQ(stop->getFrontTicket()->getTicketId(), 6);
}

TEST_F(LedgerTests, TestStopLevelHeadOrderChangeOnStopLimitOrderCancelLeavingEmptyLimit){
    ledger->addStopLimitTicket(5, true, 80, 25, 20);

    PriceLevel* stop = ledger->searchStopLevels(20);

    EXPECT_EQ(stop->getFrontTicket()->getTicketId(), 5);

    ledger->cancelStopLimitTicket(5);
    
    EXPECT_EQ(stop->getFrontTicket(), nullptr);
}

TEST_F(LedgerTests, TestCancelStopLimitOrderLeavingEmptyLimit){
    ledger->addStopLimitTicket(5, true, 80, 21, 20);
    ledger->addStopLimitTicket(6, true, 80, 20, 15);
    PriceLevel* stop1 = ledger->searchStopLevels(20);
    PriceLevel* stop2 = ledger->searchStopLevels(15);

    EXPECT_EQ(stop2->getFrontTicket()->getTicketId(), 6);
    EXPECT_EQ(stop1->getLeftChild()->getPrice(), 15);

    ledger->cancelStopLimitTicket(6);
    
    EXPECT_EQ(ledger->searchLevelMaps(15, true), nullptr);
    EXPECT_EQ(stop1->getLeftChild(), nullptr);
}

// Modifying stop imit orders tests
TEST_F(LedgerTests, TestModifyStopLimitOrderToExistingStopLevel){
    ledger->addStopLimitTicket(111, true, 10, 82, 80);
    ledger->addStopLimitTicket(112, true, 20, 83, 80);
    ledger->addStopLimitTicket(113, true, 7, 86, 85);
    ledger->addStopLimitTicket(114, true, 14, 86, 85);

    ledger->modifyStopLimitTicket(113, 40, 82, 80);

    PriceLevel* stop1 = ledger->searchStopLevels(80);
    PriceLevel* stop2 = ledger->searchStopLevels(85);

    EXPECT_EQ(stop1->getFrontTicket()->getTicketId(), 111);
    EXPECT_EQ(stop2->getFrontTicket()->getTicketId(), 114);
    EXPECT_EQ(stop1->getAggregateQty(), 70);
    EXPECT_EQ(stop2->getAggregateQty(), 14);
}

TEST_F(LedgerTests, TestModifyStopLimitOrderToNewStopLevel){
    ledger->addStopLimitTicket(111, true, 10, 81, 80);
    ledger->addStopLimitTicket(112, true, 20, 82, 80);
    ledger->addStopLimitTicket(113, true, 7, 81, 85);
    ledger->addStopLimitTicket(114, true, 14, 86, 85);

    ledger->modifyStopLimitTicket(113, 40, 83, 82);

    PriceLevel* stop1 = ledger->searchStopLevels(82);
    PriceLevel* stop2 = ledger->searchStopLevels(85);

    EXPECT_EQ(stop1->getFrontTicket()->getTicketId(), 113);
    EXPECT_EQ(stop2->getFrontTicket()->getTicketId(), 114);
    EXPECT_EQ(stop1->getAggregateQty(), 40);
    EXPECT_EQ(stop2->getAggregateQty(), 14);
}

TEST_F(LedgerTests, TestModifyStopLimitOrderInvalidOrderId){
    ledger->addStopLimitTicket(111, true, 10, 81, 80);
    ledger->addStopLimitTicket(112, true, 20, 90, 80);
    ledger->addStopLimitTicket(113, true, 7, 87, 85);
    ledger->addStopLimitTicket(114, true, 14, 87, 85);

    ledger->modifyStopLimitTicket(110, 40, 85, 82);

    PriceLevel* stop1 = ledger->searchStopLevels(80);
    PriceLevel* stop2 = ledger->searchStopLevels(85);

    EXPECT_EQ(stop1->getFrontTicket()->getTicketId(), 111);
    EXPECT_EQ(stop2->getFrontTicket()->getTicketId(), 113);
    EXPECT_EQ(stop1->getAggregateQty(), 30);
    EXPECT_EQ(stop2->getAggregateQty(), 21);
    EXPECT_EQ(ledger->searchStopLevels(82), nullptr);
    EXPECT_EQ(ledger->searchTicketRegistry(110), nullptr);
}

// Stop limit orders being triggered tests
TEST_F(LedgerTests, TestStopLimitOrdersTriggeredToMarketOrderByMarketSellOrder){
    ledger->addLimitTicket(111, true, 10, 100);
    ledger->addLimitTicket(112, true, 10, 99);
    ledger->addLimitTicket(113, true, 10, 98);

    ledger->addStopLimitTicket(114, false, 15, 97, 99);

    EXPECT_EQ(ledger->getBestBid()->getPrice(), 100);
    EXPECT_EQ(ledger->getHighestStopAsk()->getPrice(), 99);

    ledger->marketTicket(115, false, 11);

    EXPECT_EQ(ledger->getBestBid()->getPrice(), 98);
    EXPECT_EQ(ledger->getBestBid()->getAggregateQty(), 4);
    EXPECT_EQ(ledger->getHighestStopAsk(), nullptr);
}

TEST_F(LedgerTests, TestStopLimitOrdersTriggeredToMarketOrderByMarketBuyOrder){
    ledger->addLimitTicket(111, false, 10, 100);
    ledger->addLimitTicket(112, false, 10, 99);
    ledger->addLimitTicket(113, false, 10, 98);

    ledger->addStopLimitTicket(114, true, 15, 100, 99);

    EXPECT_EQ(ledger->getBestAsk()->getPrice(), 98);
    EXPECT_EQ(ledger->getLowestStopBid()->getPrice(), 99);

    ledger->marketTicket(115, true, 11);

    EXPECT_EQ(ledger->getBestAsk()->getPrice(), 100);
    EXPECT_EQ(ledger->getBestAsk()->getAggregateQty(), 4);
    EXPECT_EQ(ledger->getLowestStopBid(), nullptr);
}

TEST_F(LedgerTests, TestStopLimitOrdersPartiallyTriggeredToMarketOrderByMarketSellOrder){
    ledger->addLimitTicket(111, true, 10, 100);
    ledger->addLimitTicket(112, true, 10, 99);
    ledger->addLimitTicket(113, true, 10, 98);

    ledger->addStopLimitTicket(114, false, 15, 99, 99);

    EXPECT_EQ(ledger->getBestBid()->getPrice(), 100);
    EXPECT_EQ(ledger->getHighestStopAsk()->getPrice(), 99);

    ledger->marketTicket(115, false, 11);

    EXPECT_EQ(ledger->getBestBid()->getPrice(), 98);
    EXPECT_EQ(ledger->getBestBid()->getAggregateQty(), 10);
    EXPECT_EQ(ledger->getBestAsk()->getPrice(), 99);
    EXPECT_EQ(ledger->getBestAsk()->getAggregateQty(), 6);
    EXPECT_EQ(ledger->getHighestStopAsk(), nullptr);
}

TEST_F(LedgerTests, TestStopLimitOrdersPartiallyTriggeredToMarketOrderByMarketBuyOrder){
    ledger->addLimitTicket(111, false, 10, 100);
    ledger->addLimitTicket(112, false, 10, 99);
    ledger->addLimitTicket(113, false, 10, 98);

    ledger->addStopLimitTicket(114, true, 15, 99, 99);

    EXPECT_EQ(ledger->getBestAsk()->getPrice(), 98);
    EXPECT_EQ(ledger->getLowestStopBid()->getPrice(), 99);

    ledger->marketTicket(115, true, 11);

    EXPECT_EQ(ledger->getBestAsk()->getPrice(), 100);
    EXPECT_EQ(ledger->getBestAsk()->getAggregateQty(), 10);
    EXPECT_EQ(ledger->getBestBid()->getPrice(), 99);
    EXPECT_EQ(ledger->getBestBid()->getAggregateQty(), 6);
    EXPECT_EQ(ledger->getLowestStopBid(), nullptr);
}

TEST_F(LedgerTests, TestStopLimitOrdersTriggeredToLimitOrderByMarketSellOrder){
    ledger->addLimitTicket(111, true, 10, 100);
    ledger->addLimitTicket(112, true, 10, 99);
    ledger->addLimitTicket(113, true, 10, 98);

    ledger->addStopLimitTicket(114, false, 15, 100, 99);

    EXPECT_EQ(ledger->getBestBid()->getPrice(), 100);
    EXPECT_EQ(ledger->getHighestStopAsk()->getPrice(), 99);

    ledger->marketTicket(115, false, 11);

    EXPECT_EQ(ledger->getBestBid()->getPrice(), 99);
    EXPECT_EQ(ledger->getBestBid()->getAggregateQty(), 9);
    EXPECT_EQ(ledger->getBestAsk()->getPrice(), 100);
    EXPECT_EQ(ledger->getBestAsk()->getAggregateQty(), 15);
    EXPECT_EQ(ledger->getHighestStopAsk(), nullptr);
}

TEST_F(LedgerTests, TestStopLimitOrdersTriggeredToLimitOrderByMarketBuyOrder){
    ledger->addLimitTicket(111, false, 10, 100);
    ledger->addLimitTicket(112, false, 10, 99);
    ledger->addLimitTicket(113, false, 10, 98);

    ledger->addStopLimitTicket(114, true, 15, 98, 99);

    EXPECT_EQ(ledger->getBestAsk()->getPrice(), 98);
    EXPECT_EQ(ledger->getLowestStopBid()->getPrice(), 99);

    ledger->marketTicket(115, true, 11);

    EXPECT_EQ(ledger->getBestAsk()->getPrice(), 99);
    EXPECT_EQ(ledger->getBestAsk()->getAggregateQty(), 9);
    EXPECT_EQ(ledger->getBestBid()->getPrice(), 98);
    EXPECT_EQ(ledger->getBestBid()->getAggregateQty(), 15);
    EXPECT_EQ(ledger->getLowestStopBid(), nullptr);
}

TEST_F(LedgerTests, TestBuyStopLimitOrderWithEmptySellTree){
    ledger->addLimitTicket(111, true, 10, 100);
    ledger->addLimitTicket(112, true, 10, 99);

    ledger->addStopLimitTicket(114, false, 15, 98, 99);

    EXPECT_EQ(ledger->getBestBid()->getPrice(), 100);
    EXPECT_EQ(ledger->getHighestStopAsk()->getPrice(), 99);

    ledger->marketTicket(115, false, 11);

    EXPECT_EQ(ledger->getBestBid(), nullptr);
    EXPECT_EQ(ledger->getHighestStopAsk(), nullptr);
}

TEST_F(LedgerTests, TestSellStopLimitOrderWithEmptyBuyTree){
    ledger->addLimitTicket(112, false, 10, 99);
    ledger->addLimitTicket(113, false, 10, 98);

    ledger->addStopLimitTicket(114, true, 15, 100, 99);

    EXPECT_EQ(ledger->getBestAsk()->getPrice(), 98);
    EXPECT_EQ(ledger->getLowestStopBid()->getPrice(), 99);

    ledger->marketTicket(115, true, 11);

    EXPECT_EQ(ledger->getBestAsk(), nullptr);
    EXPECT_EQ(ledger->getLowestStopBid(), nullptr);
}

// Stop levels containing stop orders and stop limit orders tests
TEST_F(LedgerTests, TestStopOrdersAndStopLimitOrdersHeldInSameStopLevel){
    ledger->addStopLimitTicket(114, false, 15, 97, 99);
    ledger->addStopTicket(115, false, 26, 99);
    ledger->addStopLimitTicket(116, false, 5, 99, 99);
    ledger->addStopTicket(117, false, 19, 99);

    EXPECT_EQ(ledger->getHighestStopAsk()->getAggregateQty(), 65);

    ledger->addLimitTicket(111, true, 10, 100);
    ledger->addLimitTicket(112, true, 10, 99);
    ledger->addLimitTicket(113, true, 60, 98);

    ledger->marketTicket(115, false, 11);

    EXPECT_EQ(ledger->getBestBid()->getPrice(), 98);
    EXPECT_EQ(ledger->getBestBid()->getAggregateQty(), 9);
    EXPECT_EQ(ledger->getBestAsk()->getPrice(), 99);
    EXPECT_EQ(ledger->getBestAsk()->getAggregateQty(), 5);
    EXPECT_EQ(ledger->getHighestStopAsk(), nullptr);
}

TEST_F(LedgerTests, TestStopOrdersAndStopLimitOrdersMoreComplexCase){
    ledger->addLimitTicket(111, true, 480, 292);
    ledger->addLimitTicket(112, true, 353, 291);
    ledger->addLimitTicket(113, true, 108, 289);
    ledger->addLimitTicket(114, true, 49033, 288);

    ledger->addStopLimitTicket(115, false, 441, 288, 289);
    ledger->addStopLimitTicket(116, false, 1000, 287, 288);
    ledger->addStopTicket(117, false, 3000, 288);
    ledger->addStopLimitTicket(118, false, 417, 287, 288);

    EXPECT_EQ(ledger->getBestBid()->getPrice(), 292);
    EXPECT_EQ(ledger->getHighestStopAsk()->getPrice(), 289);

    ledger->marketTicket(119, false, 163);
    ledger->marketTicket(120, false, 337);
    ledger->marketTicket(121, false, 977);

    EXPECT_EQ(ledger->getBestBid()->getPrice(), 288);
    EXPECT_EQ(ledger->getBestBid()->getAggregateQty(), 43639);
    EXPECT_EQ(ledger->getHighestStopAsk(), nullptr);
}
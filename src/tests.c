#include <stdlib.h>
#include <stdio.h>

#include "dbg.h"
#include "message.h"
#include "ham.h"

/* Message class tests {{{ */
int test_Message_Header_init(void) {
    unsigned char source = (unsigned char)15;
    unsigned char destination = (unsigned char)0;
    Opcodes op = o_HEARTBEAT;
    Header* newHeader = Header_init(source, destination, op);
    check_mem(newHeader);

    check(source==newHeader->source, 
            "Sources not equal %d!=%d.", (int)source, (int)newHeader->source);
    check(destination==newHeader->destination, 
            "destinations not equal %d!=%d.", (int)destination, (int)newHeader->destination);
    check(op==newHeader->opcode, "ops not equal %d!=%d.", op, (int)newHeader->opcode);
    
    unsigned char checkSum = source + destination + (unsigned char)op;
    check(checkSum==newHeader->checkSum, 
            "Checksums not equal %d!=%d", (int)checkSum, (int)newHeader->checkSum);

    Header_destroy(newHeader);
    return(0);
error:
    if(newHeader) Header_destroy(newHeader);
    return(1);
}

int test_Message_Header_destroy(void) {
    unsigned char source = (unsigned char)15;
    unsigned char destination = (unsigned char)4;
    Opcodes op = o_VOTEYES;
    Header* newHeader = Header_init(source, destination, op);
    check_mem(newHeader);

    Header_destroy(newHeader);
    newHeader = NULL;
    // Need a better test here
    check(!(newHeader), "Um, bad...");
    return(0);
error:
    if(newHeader) Header_destroy(newHeader);
    return(1);
}

int test_Message(void) {
    int pass = 0;
    int fail = 0;

    /* Message class tests */
    printf("Testing Message Class\n");

    if(test_Message_Header_init()) {
        fail++; printf("f");
    } else {
        pass++; printf(".");
    }
    if(test_Message_Header_destroy()) {
        fail++; printf("f");
    } else {
        pass++; printf(".");
    }

    printf("\nMessage class stats: Failed=%d, Passed=%d, Total=%d\n", 
            fail, pass, fail+pass);
    return(fail);
}
/* }}} End of message class tests */

/* VoteCoord class tests {{{ */
int test_VoteCoord_Vote_Startup_Shutdown(void) {
    VoteCoord* newVoter = Vote_Startup(5);
    check_mem(newVoter);
    
    check(newVoter->systemSize == 5, "systemSize not set initiated: %d", newVoter->systemSize);
    check(newVoter->nextVoteID == 0, "nextVoteID not set to zero instead %d", newVoter->nextVoteID);
    check(newVoter->numActiveVotes == 0, "numActiveVotes not set to zero instead %d", 
            newVoter->numActiveVotes);
    check(newVoter->activeVotes, "activeVotes not set");

    Vote_Shutdown(newVoter);
    return(0);
error:
    if(newVoter) Vote_Shutdown(newVoter);
    return(1);
}

int test_VoteCoord_Init(void) {
    VoteCoord* voter = Vote_Startup(10);
    unsigned int deadID = 3;
    int commitQuorum = 3;
    int abortQuorum = 1;

    Vote_Coord_Init(voter, deadID, commitQuorum, abortQuorum);
    
    check(voter->numActiveVotes==1, "numActiveVotes not incremented");
    check(voter->nextVoteID==1, "nextVoteID not incremented");
    check_mem(voter->activeVotes);

    check(voter->activeVotes[3]->voteID==1, "voteID not set: %d", voter->activeVotes[3]->voteID);
    check(voter->activeVotes[3]->votesNay==0, "votesNay not set");
    check(voter->activeVotes[3]->votesYay==0, "votesYay not set");
    check(voter->activeVotes[3]->commitQuorum==commitQuorum, "commitQuorum not set");
    check(voter->activeVotes[3]->abortQuorum==abortQuorum, "abortQuorum not set");

    Vote_Coord_Restart_Node(voter, 0);
    Vote_Shutdown(voter);
    return(0);
error:
    return(1);
}

int test_VoteCoord_Simulate_Votes(void) {
    VoteCoord* voter = Vote_Startup(3);
    unsigned int deadID = 2;
    int commitQuorum = 3;
    int abortQuorum = 1;

    Vote_Coord_Init(voter, deadID, commitQuorum, abortQuorum);
    Vote_Coord_Yay(voter, deadID);
    Vote_Coord_Yay(voter, deadID);
    Vote_Coord_Nay(voter, deadID);

    check(voter->activeVotes[deadID]->votesNay==1, "votesNay not set");
    check(voter->activeVotes[deadID]->votesYay==2, "votesYay not set");
    check(voter->numActiveVotes==0, "Active Votes not decreased");

    deadID--;
    Vote_Coord_Init(voter, deadID, commitQuorum, abortQuorum);
    Vote_Coord_Yay(voter, deadID);
    Vote_Coord_Yay(voter, deadID);
    Vote_Coord_Yay(voter, deadID);
    check(voter->activeVotes[deadID]->votesNay==0, "votesNay not set");
    check(voter->activeVotes[deadID]->votesYay==3, "votesYay not set");
    check(voter->numActiveVotes==0, "Active Votes not decreased");

    Vote_Shutdown(voter);
    return(0);
error:
    return(1);
}

int test_VoteCoord_Simultaneous_Votes(void) {
    VoteCoord* voter = Vote_Startup(3);
    unsigned int deadID = 2;
    int commitQuorum = 3;
    int abortQuorum = 1;

    Vote_Coord_Init(voter, deadID, commitQuorum, abortQuorum);
    Vote_Coord_Yay(voter, deadID);

    check(voter->activeVotes[deadID]->votesNay==0, "votesNay not set");
    check(voter->activeVotes[deadID]->votesYay==1, "votesYay not set");
    check(voter->numActiveVotes==1, "Active Votes decreased");

    deadID--;
    Vote_Coord_Init(voter, deadID, commitQuorum, abortQuorum);
    Vote_Coord_Yay(voter, deadID);
    Vote_Coord_Yay(voter, deadID);
    Vote_Coord_Yay(voter, deadID);
    check(voter->activeVotes[deadID]->votesNay==0, "votesNay not set");
    check(voter->activeVotes[deadID]->votesYay==3, "votesYay not set");
    check(voter->numActiveVotes==1, "Active Votes not decreased");

    deadID++;
    Vote_Coord_Yay(voter, deadID);
    Vote_Coord_Nay(voter, deadID);
    check(voter->activeVotes[deadID]->votesNay==1, "votesNay not set");
    check(voter->activeVotes[deadID]->votesYay==2, "votesYay not set");
    check(voter->numActiveVotes==0, "Active Votes not decreased");

    Vote_Shutdown(voter);
    return(0);
error:
    return(1);
}

int test_VoteCoord(void) {
    int pass = 0;
    int fail = 0;

    printf("Testing Vote Coordinator Class\n");

    if(test_VoteCoord_Vote_Startup_Shutdown()) {
        fail++; printf("f");
    } else {
        pass++; printf(".");
    }
    if(test_VoteCoord_Init()) {
        fail++; printf("f");
    } else {
        pass++; printf(".");
    }
    if(test_VoteCoord_Simulate_Votes()) {
        fail++; printf("f");
    } else {
        pass++; printf(".");
    }
    if(test_VoteCoord_Simultaneous_Votes()) {
        fail++; printf("f");
    } else {
        pass++; printf(".");
    }

    printf("\nVote Coordinator class stats: Failed=%d, Passed=%d, Total=%d\n", 
            fail, pass, fail+pass);
    return(fail);
}
/* }}} End of message class tests */

int main(int argc, char* argv[]) {
    int fails = 0;
    if(argc>1) { // Test specific class
        if(!strcmp(argv[1],"Message")) {
            fails += test_Message();
        } else if(!strcmp(argv[1],"VoteCoord")) {
            fails += test_VoteCoord();
        } else {
            printf("Test not recogonized!\n");
            return(-1);
        }
    } else { // Test all
        printf("-------------------------------------------------------------\n");
        fails += test_Message();
        printf("-------------------------------------------------------------\n");
        fails += test_VoteCoord();
        printf("-------------------------------------------------------------\n");
    }

    return(fails);
}


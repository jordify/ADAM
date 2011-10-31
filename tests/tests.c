#include <stdlib.h>
#include <stdio.h>

#include "../src/dbg.h"
#include "../src/message.h"

/* Message class tests {{{*/
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
    check((newHeader), "Um, bad...");
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

    printf("\nMessage class Stats: Failed=%d, Passed=%d, Total=%d\n", 
            fail, pass, fail+pass);
    return(fail);
}
/* }}} End of message class tests */

int main(int argc, char* argv[]) {
    if(argc>1) { // Test specific class
        if(!strcmp(argv[1],"Message")) {
            test_Message();
        } else {
            printf("Test not recogonized!\n");
            return(-1);
        }
    } else { // Test all
        test_Message();
    }

    return(0);
}


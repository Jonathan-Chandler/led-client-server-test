#include "debug.h"





void dbg_verbose_print_vector(const std::vector<uint8_t> &client_message)
{
    // byte count
    fprintf(stderr, "%s::%d - %s(VERBOSE) -                         PRINT VECTOR\n",__FILE__,__LINE__,__FUNCTION__);
    fprintf(stderr, "%s::%d - %s(VERBOSE) - ",__FILE__,__LINE__,__FUNCTION__);
    fprintf(stderr, "    ");
    for (int x = 0; x < 0x10; x++)
    {
        fprintf(stderr, "%02X ", (x & 0xFF));
    }
    fprintf(stderr, "\n");

    // value
    for (int x = 0; x < client_message.size(); x+=0x10)
    {
        fprintf(stderr, "%s::%d - %s(VERBOSE) - %02X: ",__FILE__,__LINE__,__FUNCTION__, (x & 0xFF));
        for (int y = 0; (y < 0x10) && ((x+y) < client_message.size()); y++)
        {
            fprintf(stderr, "%02X ", (client_message[x+y] & 0xFF) );
        }

        fprintf(stderr, "\n");
    }
}

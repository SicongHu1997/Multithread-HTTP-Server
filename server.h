/*
struct for sending socket and path to web root
to the function called by pthread_create
*/
struct arg_struct {
    int arg1;
    char arg2[1024];
};
//the function called by pthread_create, handle_request
void handle_request(void *arguments);
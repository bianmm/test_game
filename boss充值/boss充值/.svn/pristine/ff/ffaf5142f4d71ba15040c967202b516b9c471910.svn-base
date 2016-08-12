//-----------------------------------//
//-----------------------------------//

#ifndef _MY_EXCEPTION_H_MINGLIN_XUU_080722_
#define _MY_EXCEPTION_H_MINGLIN_XUU_080722_

//-----------------------------------//
//-----------------------------------//

#define _MAX_EXCEPTION_MESSAGE_LENGTH  4096

//-----------------------------------//
//-----------------------------------//

class c_my_exception_t
{
public:
    c_my_exception_t();
    c_my_exception_t(int exception_code,const char* sz_exception_message);
    ~c_my_exception_t();
    
    int get_exception_code();
    int get_exception_message(char* p_recv_buffer,int buffer_len);
    const char* get_exception_message();
    
private:
    int m_exception_code;
    char m_sz_exception_message[_MAX_EXCEPTION_MESSAGE_LENGTH + 1];
};

//-----------------------------------//
//-----------------------------------//

#endif//_MY_EXCEPTION_H_MINGLIN_XUU_080722_

//-----------------------------------//
//-----------------------------------//

#include <stdbool.h>

syscall_hdlr_1(uint32_t, getDentsSize, "0", char*, path);
syscall_hdlr_2(void, getDents, "1", char*, path, uint8_t*, buffer);
syscall_hdlr_1(bool, exists, "2", char*, path);
syscall_hdlr_1(FSType, type, "3", char*, path);
syscall_hdlr_1(void*, malloc, "4", uint32_t, size);
syscall_hdlr_1(void, free, "5", void*, ptr);
syscall_hdlr_0(uint32_t, getPID, "8");
syscall_hdlr_3(int, read, "10", uint32_t, count, uint32_t, filehandler, uint8_t*, buffer);
syscall_hdlr_3(int, write, "11", uint32_t, count, uint32_t, filehandler, const uint8_t*, buffer);
syscall_hdlr_1(uint32_t, exec, "7", uint32_t, filehandler);
syscall_hdlr_1(uint32_t, bindStdout, "13", uint32_t, PID);
syscall_hdlr_1(uint32_t, bindStdin, "14", uint32_t, PID);
syscall_hdlr_1(uint32_t, fopen, "15", char*, filename);
syscall_hdlr_1(void, fclose, "16", uint32_t, filehandler);
syscall_hdlr_1(void, kill, "17", uint32_t, PID);
syscall_hdlr_1(void, sleep, "18", uint32_t, time);
syscall_hdlr_0(void, bindToKeyboard, "12");
SOURCE = $(wildcard *.c)

OBJS = $(patsubst %.c,%.o,$(SOURCE))

Rtsp_Server_Phoenix: $(OBJS)

	gcc -g -Wall -O2 -o $@ $^ -lpthread 
#makefile的隐含规则会自动推导$(OBJS)中的各个“.o“目标文件的依赖规则，然后自动展开                                \
但是，自动展开后的形式为 gcc -c server_main.c 即隐含规则不会加入-g -Wall -O2等编译选项          \
如果想在隐含规则中不丢弃编译选项可以：CFLAGS =-g -Wall -O2                           \
                              gcc $(CFLAGS)-o Rtsp_Server_Phoenix $(OBJS) -lpthread \
这样在隐含规则中展开后得到：gcc -c $(CFLAGS) server_main.c 

.PHONY: clean
clean:
	rm -f *.o Rtsp_Server_Phoenix
	

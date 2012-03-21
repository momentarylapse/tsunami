# module: net

NET_BIN  = temp/net.a
NET_DIR  = lib/net/
NET_OBJ  = temp/net.o
NET_CXXFLAGS =  $(GLOBALFLAGS)


$(NET_BIN) : $(NET_OBJ)
	rm -f $@
	ar cq $@ $(NET_OBJ)

temp/net.o : $(NET_DIR)net.cpp
	$(CPP) -c $(NET_DIR)net.cpp -o $@ $(NET_CXXFLAGS)

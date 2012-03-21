# module: image

IMAGE_BIN  = temp/image.a
IMAGE_OBJ  = temp/image.o temp/image_bmp.o temp/image_tga.o temp/image_jpg.o
IMAGE_CXXFLAGS = $(GLOBALFLAGS)

$(IMAGE_BIN) : $(IMAGE_OBJ)
	rm -f $@
	ar cq $@ $(IMAGE_OBJ)

temp/image.o : image/image.cpp
	$(CPP) -c image/image.cpp -o $@ $(IMAGE_CXXFLAGS)

temp/image_bmp.o : image/image_bmp.cpp
	$(CPP) -c image/image_bmp.cpp -o $@ $(IMAGE_CXXFLAGS)

temp/image_tga.o : image/image_tga.cpp
	$(CPP) -c image/image_tga.cpp -o $@ $(IMAGE_CXXFLAGS)

temp/image_jpg.o : image/image_jpg.cpp
	$(CPP) -c image/image_jpg.cpp -o $@ $(IMAGE_CXXFLAGS)

FLAGS = -faligned-new -fcf-protection=none -Wno-bool-operation -Wno-sign-compare -Wno-uninitialized -Wno-unused-but-set-variable -Wno-unused-parameter -Wno-unused-variable -Wno-shadow

EXPORT_NAME = rv32_cache


default: build

build:  main.cpp cache.cpp
	mkdir -p obj_dir/
	g++ -I. $(FLAGS) -O0 -g -o obj_dir/$(EXPORT_NAME) $^ -lm -lstdc++ -lsystemc

run: build
	chmod +x obj_dir/$(EXPORT_NAME)
	obj_dir/$(EXPORT_NAME)

clean:
	rm -rf obj_dir/*

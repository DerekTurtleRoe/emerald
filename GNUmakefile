name := amethyst
build := stable
flags += -I.

nall.path := ./nall
include $(nall.path)/GNUmakefile

hiro.path := ./hiro
include $(hiro.path)/GNUmakefile

objects := obj/$(name).o

obj/$(name).o: $(name).cpp

all: $(hiro.objects) $(objects)
	$(info Linking out/$(name) ...)
	+@$(compiler) -o out/$(name) $(hiro.objects) $(objects) $(hiro.options) $(options)

verbose: hiro.verbose nall.verbose all;

clean:
	$(call delete,obj/*)
	$(call delete,out/*)

install: all
	mkdir -p $(prefix)/bin/
	mkdir -p $(prefix)/share/applications/
	mkdir -p $(prefix)/share/icons/
	mkdir -p $(prefix)/share/$(name)/
	cp out/$(name) $(prefix)/bin/$(name)
	cp data/$(name).desktop $(prefix)/share/applications/$(name).desktop
	cp data/$(name).svg $(prefix)/share/icons/$(name).svg
	cp data/*.bml $(prefix)/share/$(name)/

uninstall:
	rm -f $(prefix)/bin/$(name)

-include obj/*.d

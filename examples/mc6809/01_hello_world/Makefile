PROJECT_NAME = hello_world

all: $(PROJECT_NAME).decb

$(PROJECT_NAME).decb: base.s ../../../rom/mc6809/definitions.i main.s
	asm6809 -9 -B -o $(PROJECT_NAME).decb -l $(PROJECT_NAME).list base.s

clean:
	rm $(PROJECT_NAME).decb $(PROJECT_NAME).list

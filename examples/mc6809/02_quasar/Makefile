PROJECT_NAME = quasar

all: $(PROJECT_NAME).decb

$(PROJECT_NAME).decb: base.s ../../../rom/mc6809/definitions.i tiles.s main.s
	asm6809 -9 -B -o $(PROJECT_NAME).decb -l $(PROJECT_NAME).list base.s

clean:
	rm $(PROJECT_NAME).decb $(PROJECT_NAME).list

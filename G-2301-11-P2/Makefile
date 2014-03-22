CC = clang
CFLAGS :=  -Wall -pedantic -std=c99 -D_GNU_SOURCE $(shell pkg-config --cflags gtk+-2.0)
DEBUG_CFLAGS = -DDEBUG -ggdb 
LDFLAGS := $(shell pkg-config --libs gtk+-2.0)
E_LIBS = -lpthread

OBJDIR = obj
SRCDIR = src
INCDIR = includes
BINDIR = bin
DOCDIR = doc
LIBDIR = lib
LIBSRCDIR = srclib
TESTDIR = tests

P_GROUP = 2301
P_NUM = 2
P_PAR = 11
P_NAME = G-$(P_GROUP)-$(P_PAR)-P$(P_NUM)

TARGET = main
TARGETS := main chat prsound
INCLUDES = -I./includes/

INCS := $(wildcard $(INCDIR)/*.h)
SRCS := $(wildcard $(SRCDIR)/*.c)
OBJS := $(addprefix $(OBJDIR)/, $(patsubst %.c,%.o, $(SRCS)))

LIB_SRCS := $(wildcard $(LIBSRCDIR)/**/*.c)
LIB_OBJS := $(addprefix $(OBJDIR)/, $(patsubst %.c,%.o, $(LIB_SRCS)))
LIB_NAMES := $(patsubst $(LIBSRCDIR)/%, %, $(wildcard $(LIBSRCDIR)/*))
LIBS :=	$(addprefix $(LIBDIR)/, $(addsuffix .a, $(LIB_NAMES)))
LIB_DEPS := $(addsuffix .deps, $(addprefix $(OBJDIR)/., $(LIB_NAMES)))
LIB_OBJDIRS := $(addprefix $(OBJDIR)/$(LIBSRCDIR)/, $(LIB_NAMES))

TEST_SRCS := $(wildcard $(TESTDIR)/*.c)
TEST_OBJS := $(addprefix $(OBJDIR)/, $(patsubst %.c,%.o, $(TEST_SRCS)))
OBJS_NOMAIN := $(filter-out $(addprefix %/, $(addsuffix .o, $(TARGETS))), $(OBJS))

DOC_TEXS := $(wildcard $(DOCDIR)/*.tex)
DOC_TEXS += $(DOCDIR)/latex/refman.tex
DOC_PDFS := $(patsubst %.tex, %.pdf, $(DOC_TEXS))

DOXY_OUTPUTS := $(filter-out $(DOCDIR)/, $(dir $(wildcard $(DOCDIR)/*/)))

FMT_BOLD := $(shell tput bold)
FMT_NORM := $(shell tput sgr0)

.PRECIOUS: %.o %.d
.PHONY: debug clean pack doxydoc docclean
 
all: debug test

final: all docs pack

### Compilation

debug: CFLAGS += $(DEBUG_CFLAGS)
debug: $(TARGET)

no-daemon: CFLAGS += -DNODAEMON
no-daemon: debug

chat: CFLAGS += $(DEBUG_CFLAGS)
chat: LDFLAGS += 
chat: $(BINDIR)/chat

prsound: LDFLAGS += $(shell pkg-config --libs libpulse-simple)
prsound: $(BINDIR)/prsound

scan: clean
	@scan-build make test
	
$(OBJS): | $(OBJDIR)

## Dependencies

depend: $(OBJDIR)/.deps $(LIB_DEPS)

$(OBJDIR)/.deps: $(SRCS) | $(OBJDIR)
	@-rm -f $(OBJDIR)/.deps
	@$(CC) $(CFLAGS) $(INCLUDES) -MM $^ >> $(OBJDIR)/.deps;
	@awk '{if (sub(/\\$$/,"")) printf "%s", $$0; else print $$0}' $@ > "$@.0"
	@mv "$@.0" $@
	@awk '{printf("$(OBJDIR)/$(SRCDIR)/%s\n", $$0)}' $@ > "$@.0"
	@mv "$@.0" $@

$(OBJDIR)/.%.deps: $(LIBSRCDIR)/%/*.c | $(OBJDIR)
	@-rm -f $@
	@$(CC) $(CFLAGS) $(INCLUDES) -MM $^ >> $@;
	@awk '{if (sub(/\\$$/,"")) printf "%s", $$0; else print $$0}' $@ > "$@.0"
	@mv "$@.0" $@
	@awk '{printf("$(OBJDIR)/$(LIBSRCDIR)/$*/%s\n", $$0)}' $@ > "$@.0"
	@mv "$@.0" $@
	@echo $(LIBDIR)/$*.a: $(patsubst %.c, $(OBJDIR)/%.o, $^) >> $@

-include $(OBJDIR)/.deps
-include $(LIB_DEPS)

## Directories

$(OBJDIR):
	@echo Creating obj directories
	@mkdir -p $(OBJDIR)/$(SRCDIR)
	@mkdir -p $(OBJDIR)/$(LIBSRCDIR)
	@mkdir -p $(OBJDIR)/$(TESTDIR)
	@mkdir -p $(LIB_OBJDIRS)

$(BINDIR):
	@echo Creating bin directory
	@mkdir -p $(BINDIR)

$(LIBDIR):
	@echo Creating lib directory
	@mkdir -p $(LIBDIR)

## Common

$(OBJDIR)/%.o: %.c | $(OBJDIR) depend
	@echo "$< -> $@"
	@$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

clean: docclean
	@echo Clearing build folders...
	-rm -rf $(OBJDIR) $(BINDIR) $(LIBDIR)
	
## Libraries

$(LIBDIR)/%.a: | $(LIBDIR) $(OBJDIR)/$(LIBSRCDIR)/%/
	@echo "$(FMT_BOLD)Building library $*... $(FMT_NORM)"
	@$(AR) -r $@ $?

## Executable

$(TARGET): $(BINDIR)/$(TARGET)

$(BINDIR)/%: $(OBJDIR)/$(SRCDIR)/%.o $(OBJS_NOMAIN) $(LIBS) | $(BINDIR) depend $(LIBS)
	@echo "$(FMT_BOLD)Building final target: $@ $(FMT_NORM)"
	@$(CC) $(CFLAGS) $(LDFLAGS) $(INCLUDES) -o $@ $^ $(E_LIBS)

## Tests

test: CFLAGS += $(DEBUG_CFLAGS)
test: $(BINDIR)/test | $(OBJDIR)

$(BINDIR)/test: $(OBJDIR)/$(TESTDIR)/test.o $(TEST_OBJS) $(OBJS_NOMAIN) $(LIBS) | $(BINDIR) depend $(LIBS)
	@echo "$(FMT_BOLD)Building test executable $(FMT_NORM)"
	@$(CC) $(CFLAGS) $(LDFLAGS) $(INCLUDES) -o $@ $^ $(E_LIBS)

### Documentation

docs: doxydoc $(DOC_PDFS)

# Por alguna razón, los manuales no tienen la codificación correcta. Por eso el for ese de ahí.
doxydoc: 
	@echo "$(FMT_BOLD)Building doxygen docs... $(FMT_NORM)"
	@doxygen
	@echo "Converting files to correct encoding..."
	@for f in doc/man/man3/*; do \
		iconv -t latin1 "$$f" > iconv.tmp; \
		mv iconv.tmp "$$f"; \
	done

%.pdf: %.tex
	@echo "$(FMT_BOLD)Latexmk: generating $@ $(FMT_NORM)"
	@cd $(dir $<); latexmk -pdf -silent $(notdir $<)

docclean:
	@echo Removing LaTeX files...
	@cd $(DOCDIR); (which latexmk > /dev/null && latexmk -C) || true # Clean tex files if latexmk is present.
	@echo Removing doxygen output...
	@-rm -r $(DOXY_OUTPUTS)
	
### Packing

pack: docs
	cp -r "$(shell pwd)" ../$(P_NAME)-backup
	@tools/redes_rename
	@cd ..; tar -X $(P_NAME)/.tarexcludes -czf $(P_NAME).tar.gz $(P_NAME) 
	@echo "Packed $(P_NAME).tar.gz in parent directory."

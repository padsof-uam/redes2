GCC = gcc
CFLAGS = -ggdb -Wall -pedantic -ansi
E_LIBS = 
TARGET = daemonize
LIB = libredes2.a
INCLUDES = -I./includes/

OBJDIR = obj
SRCDIR = src
INCDIR = includes
BINDIR = bin
DOCDIR = doc
LIBDIR = lib
LIBSRCDIR = srclib
TESTDIR = tests

INCS := $(wildcard $(INCDIR)/*.h)
SRCS := $(wildcard $(SRCDIR)/*.c)
OBJS := $(addprefix $(OBJDIR)/, $(patsubst %.c,%.o, $(SRCS)))

LIB_SRCS := $(wildcard $(LIBSRCDIR)/*.c)
LIB_OBJS := $(addprefix $(OBJDIR)/, $(patsubst %.c,%.o, $(LIB_SRCS)))

TEST_SRCS := $(wildcard $(TESTDIR)/*.c)
TEST_OBJS := $(addprefix $(OBJDIR)/, $(patsubst %.c,%.o, $(TEST_SRCS)))

DOC_TEXS := $(wildcard $(DOCDIR)/*.tex)
DOC_PDFS := $(patsubst %.tex, %.pdf, $(DOC_TEXS))

P_GROUP = 2301
P_NUM = 1
P_PAR = 11
P_NAME = G-$(P_GROUP)-$(P_PAR)-P$(P_NUM)

TAR_EXCLUDES= bin Makefile obj doc .tar.gz
TAR_EXCLUDES_ARG = $(addprefix --exclude , $(TAR_EXCLUDES))

.PRECIOUS: %.o %.d
.PHONY: debug clean pack
 
all: debug docs pack

### Compilation

debug: $(TARGET)
scan: clean
	@scan-build make
$(OBJS): | $(OBJDIR)

## Dependencies

depend: $(OBJDIR)/.deps

$(OBJDIR)/.deps: $(SRCS) $(LIB_SRCS) | $(OBJDIR)
	@-rm -f $(OBJDIR)/.deps
	@$(GCC) $(CFLAGS) $(INCLUDES) -MM $^ >> $(OBJDIR)/.deps;
	@awk '{printf("$(OBJDIR)/$(SRCDIR)/%s\n", $$0)}' $@ > "$@.0"
	@mv "$@.0" $@

-include $(OBJDIR)/.deps

## Directories

$(OBJDIR):
	@echo Creating obj directories
	@mkdir -p $(OBJDIR)/$(SRCDIR)
	@mkdir -p $(OBJDIR)/$(LIBSRCDIR)
	@mkdir -p $(OBJDIR)/$(TESTDIR)

$(BINDIR):
	@echo Creating bin directory
	@mkdir -p $(BINDIR)

$(LIBDIR):
	@echo Creating lib directory
	@mkdir -p $(LIBDIR)

## Common

$(OBJDIR)/%.o: %.c | $(OBJDIR) depend
	@echo "$< -> $@"
	@$(GCC) $(CFLAGS) $(INCLUDES) -c $< -o $@

clean: 
	-rm -rf $(OBJDIR) $(BINDIR) $(LIBDIR)
	@cd $(DOCDIR); latexmk -C # Clean tex files.

## Libraries

libr: $(LIBDIR)/$(LIB)
	@echo $(LIB_OBJS)
lib/%: $(LIB_OBJS) | $(LIBDIR)
	@echo $(LIB_OBJS)
	@echo Creating library $@
	@ar -r $@ $^ 

## Executable

$(TARGET): $(BINDIR)/$(TARGET)

$(BINDIR)/%: $(OBJDIR)/$(SRCDIR)/%.o $(OBJS) $(INCS) | $(BINDIR) depend
	@echo Building final target: $@
	@$(GCC) $(CFLAGS) $(INCLUDES) $(LIBDIR)/$(LIB) -o $@ $^ $(E_LIBS)

## Tests

test: $(BINDIR)/test

$(BINDIR)/test: $(OBJDIR)/$(TESTDIR)/test.o $(TEST_OBJS) $(INCS) | $(BINDIR) depend
	@echo Building test executable
	@$(GCC) $(CFLAGS) $(INCLUDES) $(LIBDIR)/$(LIB) -o $@ $^ $(E_LIBS)

### Documentation

docs: $(DOC_PDFS)

%.pdf: %.tex
	@echo Latexmk: generating $@
	cd $(DOCDIR); latexmk -pdf -silent $(notdir $<)

### Packing

pack: ls $(DOC_PDFS)
	@cp Makefile $(P_NAME)-makefile
	@cd ..; tar $(TAR_EXCLUDES_ARG) -czf $(P_NAME).tar.gz $(P_NAME) 
	@echo "Packed $(P_NAME).tar.gz in parent directory."
	@rm $(P_NAME)-makefile

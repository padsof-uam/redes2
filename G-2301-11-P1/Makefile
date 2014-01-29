GCC = gcc
CFLAGS = -ggdb -Wall -pedantic -ansi
LIBS = 
TARGET = daemonize
INCLUDES = -I./includes/

OBJDIR = obj
SRCDIR = src
INCDIR = includes
BINDIR = ./

INCS := $(wildcard $(INCDIR)/*.h)
SRCS := $(wildcard $(SRCDIR)/*.c)
OBJS := $(addprefix $(OBJDIR)/, $(patsubst %.c,%.o, $(SRCS)))


.PRECIOUS: %.o %.d
.PHONY: debug clean 
 
debug: $(TARGET)
scan: clean
	@scan-build make
$(OBJS): | $(OBJDIR)

depend: $(OBJDIR)/.deps

$(OBJDIR)/.deps: $(SRCS) $(INCS) | $(OBJDIR)
	@-rm -f $(OBJDIR)/.deps
	@$(CC) $(CFLAGS) $(INCLUDES) -MM $^ >> $(OBJDIR)/.deps;
	@awk '{printf("$(OBJDIR)/$(SRCDIR)/%s\n", $$0)}' $@ > "$@.0"
	@mv "$@.0" $@

-include $(OBJDIR)/.deps

$(TARGET): $(OBJDIR)/$(SRCDIR)/$(TARGET).o $(OBJS) $(INCS) | $(BINDIR) depend
	@echo Building final target: $@
	@$(GCC) $(CFLAGS) $(INCLUDES) -o $(BINDIR)/$@ $^ $(LIBS)
$(OBJDIR)/%.o: %.c | $(OBJDIR) depend
	@echo "$< -> $@"
	@$(GCC) $(CFLAGS) $(INCLUDES) -c $< -o $@
$(OBJDIR):
	@echo Creating obj directories
	@mkdir -p $(OBJDIR)/$(SRCDIR)
$(BINDIR):
	@echo Creating bin directory
	@mkdir $(BINDIR)
clean: 
	-rm -rf $(OBJDIR) $(BINDIR) $(TARGET) 

GCC = gcc
CFLAGS = -ggdb -Wall -pedantic -ansi
LIBS = 
TARGET = daemonize
INCLUDES = -I./includes/

OBJDIR = obj
SRCDIR = src
INCDIR = includes
BINDIR = bin

INCS := $(wildcard $(INCDIR)/*.h)
SRCS := $(wildcard $(SRCDIR)/*.c)
OBJS := $(addprefix $(OBJDIR)/, $(patsubst %.c,%.o, $(SRCS)))

P_GROUP = 2301
P_NUM = 1
P_PAR = 11
P_NAME = G-$(P_GROUP)-$(P_PAR)-P$(P_NUM)

.PRECIOUS: %.o %.d
.PHONY: debug clean 
 
debug: $(TARGET)
scan: clean
	@scan-build make
$(OBJS): | $(OBJDIR)

pack: 
	@cp Makefile $(P_NAME)-makefile
	@cd ..; tar --exclude bin --exclude Makefile --exclude obj --exclude .tar.gz -czf $(P_NAME).tar.gz $(P_NAME) 
	@echo "Packed $(P_NAME).tar.gz in parent directory."
	@rm $(P_NAME)-makefile

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
	@echo Creating obj directoriesg
	@mkdir -p $(OBJDIR)/$(SRCDIR)
$(BINDIR):
	@echo Creating bin directory
	@mkdir -p $(BINDIR)
clean: 
	-rm -rf $(OBJDIR) $(BINDIR) $(TARGET) 

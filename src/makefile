IDIR=../include
CC=g++-9.2
CPPFLAGS=-I$(IDIR)

ODIR=obj

_DEPS=ioreq_t.h ioscheduler.h
DEPS=$(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJ=ioscheduler.o iosched.o
OBJ=$(patsubst %,$(ODIR)/%,$(_OBJ))

$(ODIR)/%.o: %.cpp $(DEPS)
	$(CC) -c -o $@ $< $(CPPFLAGS)

iosched: $(OBJ)
	$(CC) -o $@ $^ $(CPPFLAGS)

clean:
	rm -f $(ODIR)/*.o *~ core $(INCDIR)/*~
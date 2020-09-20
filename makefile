CXX			= gcc

CXXFLAGS 	= -g -Wall

LIBS		= -lncursesw

OUTPUT		= thematrix
OBJS		= thematrix.c

$(OUTPUT): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) $(LIBS) -o $(OUTPUT)

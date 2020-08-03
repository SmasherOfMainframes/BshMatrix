CXX			= gcc

CXXFLAGS 	= -g -Wall

OUTPUT		= thematrix
OBJS		= thematrix.c

$(OUTPUT): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $(OUTPUT)

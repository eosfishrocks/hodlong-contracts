EOSIOCPP = eosiocpp

build : users trackers marketplace


marketplace:
	$(EOSIOCPP) -o ./contracts/Marketplace.wast ./contracts/Marketplace.cpp && \
		$(EOSIOCPP) -g ./contracts/Marketplace.abi ./contracts/Marketplace.cpp
trackers :
	$(EOSIOCPP) -o ./contracts/Trackers.wast ./contracts/Trackers.cpp && \
		$(EOSIOCPP) -g ./contracts/Trackers.abi ./contracts/Trackers.cpp

users :
	$(EOSIOCPP) -o ./contracts/Users.wast ./contracts/Users.cpp && \
		$(EOSIOCPP) -g ./contracts/Users.abi ./contracts/Users.cpp
		
clean :
	-find . -type f \( -name '*.wasm' -o -name '*.wast' -o -name '*.abi' -a ! -name Token.abi \) -exec rm {} +

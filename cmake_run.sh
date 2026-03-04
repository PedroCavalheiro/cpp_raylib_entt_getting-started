# This will configure cmake to put the stuff into the ./build folder
# Then will build the program
# Then will run the program
cmake -B build \
  && cmake --build build \
  && ./build/main

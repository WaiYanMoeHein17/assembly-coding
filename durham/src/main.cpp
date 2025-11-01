#include "main.h"
#include "tokenizer.h"
#include "parser.h"
#include "gen_asm.h"

//using namespace std;

void ask_user_info(struct User& user) {
    std::string college;
    std::string course; 
    char gender; 
    std::cout << "Which college are you from: "; 
    std::cin >> college; 
    std::cout << "Which course do you take: "; 
    std::cin >> course; 
    std::cout << "Gender? "; 
    std::cin >> gender; 
    user.college = college;
    user.course = course; 
    user.gender = gender; 
}

void make_jokes(User user) {

}

void autocorrect(int argc, char** argv) {

}

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "Incorrect Usage" << std::endl; 
        std::cerr << "Correct Usage: durham <input.dur>" << std::endl;  // Changed
        return EXIT_FAILURE;  // Fixed: should be FAILURE not SUCCESS
    }
    //std::cout << argv[1] << std::endl; 
    //std::cout<< "Hello World" << std::endl;

    std::ifstream input(argv[1]); // ifstream ONLY input
    
    if (!input.is_open()) {
        std::cerr << "Error: Could not open file " << argv[1] << std::endl; 
        return EXIT_FAILURE; 
    }

    std::stringstream buffer; 
    buffer << input.rdbuf(); 
    std::string contents = buffer.str(); 

    input.close(); // always close after opening file

    Tokenizer tokenizer(contents); 
    std::vector<Token> tokens = tokenizer.tokenize(); 

    Parser parser(tokens);
    std::shared_ptr<ASTNode> ast = parser.parse();
    
    std::string assembly_code = generate_assembly_from_ast(ast);

    // Write assembly
    std::ofstream output("output.asm");
    if (output.is_open()) {
        output << assembly_code;
        output.close();
        //std::cout << "Generated output.asm" << std::endl;
    } else {
        std::cerr << "Error: Could not write output.asm" << std::endl;
        return EXIT_FAILURE;
    }

    // Automatically assemble and link
    //std::cout << "Assembling..." << std::endl;
    int result = system("nasm -f win64 output.asm -o output.obj");
    if (result != 0) {
        std::cerr << "Assembly failed" << std::endl;
        return EXIT_FAILURE;
    }

    //std::cout << "Linking..." << std::endl;
    result = system("gcc output.obj -o output.exe");
    if (result != 0) {
        std::cerr << "Linking failed" << std::endl;
        return EXIT_FAILURE;
    }

    //std::cout << "Running program..." << std::endl;
    result = system("output.exe");
    
    return result;
}
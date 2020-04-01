#include <iostream>
#include <exception>
#include <vector>
#include <string>
#include <stdio.h>
#include <unistd.h>

/*
token:
    integer
    punctuator

integer:
    nonzero_digit {digit}

digit:
    0 1 2 3 4 5 6 7 8 9

nonzero_digit:
    1 2 3 4 5 6 7 8 9

punctuator:
    + - * / % ( )


additive_expression:
    multicative_expression
    additive_expression + multicative_expression
    additive_expression - multicative_expression

multicative_expression:
    primary_expression
    multicative_expression * primary_expression
    multicative_expression / primary_expression
    multicative_expression % primary_expression

primary_expression:
    integer
    (additive_expression)
*/

enum TokenType{
    TOKEN_INTEGER,
    TOKEN_PUNCTUATOR,
};

struct Token{
    TokenType type;
    std::string data;
};

static void tokenize(std::vector<Token>& tokens,const std::string& src);
static bool skip_whitespace(const std::string& src,size_t& offset);
static bool tokenize_integer(std::vector<Token>& tokens,const std::string& src,size_t& offset);
static bool tokenize_punctuator(std::vector<Token>& tokens,const std::string& src,size_t& offset);
static bool check_nonzero_digit(const std::string& src,size_t offset);
static bool check_digit(const std::string& src,size_t offset);

static void tokenize(std::vector<Token>& tokens,const std::string& src){
    size_t offset = 0;
    while (offset < src.size()){
        if (skip_whitespace(src,offset)){
            continue;
        }
        if (tokenize_integer(tokens,src,offset)){
            continue;
        }
        if (tokenize_punctuator(tokens,src,offset)){
            continue;
        }
        std::cout << "failed to tokenize" << "\n";
        std::terminate();
    }
}

static bool skip_whitespace(const std::string& src,size_t& offset){
    //check access range
    if (offset >= src.size()){
        return false;
    }

    //skip ' ' or '\n'
    if (src[offset] == ' ' || src[offset] == '\n'){
        offset++;
        return true;
    }else{
        return false;
    }
}

static bool tokenize_integer(std::vector<Token>& tokens,const std::string& src,size_t& offset){
    //read nonzero digit
    Token token;
    if (check_nonzero_digit(src,offset)){
        token.data += src[offset];
        offset++;
    }else{
        return false;
    }

    //read digit
    while (check_digit(src,offset)){
        token.data += src[offset];
        offset++;
    }

    //success
    token.type = TOKEN_INTEGER;
    tokens.push_back(token);
    return true;
}

static bool tokenize_punctuator(std::vector<Token>& tokens,const std::string& src,size_t& offset){
    //check access range
    if (offset >= src.size()){
        return false;
    }

    //read punctuator
    if (src[offset] == '+' ||
        src[offset] == '-' ||
        src[offset] == '*' ||
        src[offset] == '/' ||
        src[offset] == '%' ||
        src[offset] == '(' ||
        src[offset] == ')')
    {
        Token token;
        token.type = TOKEN_PUNCTUATOR;
        token.data += src[offset];
        tokens.push_back(token);
        offset++;
        return true;
    }else{
        return false;
    }
}

static bool check_nonzero_digit(const std::string& src,size_t offset){
    //check access range
    if (offset >= src.size()){
        return false;
    }

    //check nonzero digit
    if (src[offset] >= '1' && src[offset] <= '9'){
        return true;
    }else{
        return false;
    }
}

static bool check_digit(const std::string& src,size_t offset){
    //check access range
    if (offset >= src.size()){
        return false;
    }

    //check digit
    if (src[offset] >= '0' && src[offset] <= '9'){
        return true;
    }else{
        return false;
    }
}




/*
enum NodeType{
    NODE_ADDITIVE_EXPRESSION,
    NODE_MULTICATIVE_EXPRESSION,
    NODE_PRIMARY_EXPRESSION,
};
*/

struct Node{
    Node();
    Node(Node* c1,Node* c2,const Token* t);
    ~Node();
    //for internal node
    Node* child[2];

    //common
    const Token* token;
};

Node::Node(){
    child[0] = nullptr;
    child[1] = nullptr;
    token = nullptr;
}

Node::Node(Node* c1,Node* c2,const Token* t){
    child[0] = c1;
    child[1] = c2;
    token = t;
}

Node::~Node(){
    delete child[0];
    delete child[1];
}

static Node* parse_additive_expression(const std::vector<Token>& tokens,size_t& offset);
static Node* parse_multicative_expression(const std::vector<Token>& tokens,size_t& offset);
static Node* parse_primary_expression(const std::vector<Token>& tokens,size_t& offset);
static bool check_token(const std::string& data,const std::vector<Token>& tokens,size_t offset);
static bool check_token(TokenType type,const std::vector<Token>& tokens,size_t offset);

static Node* parse(const std::vector<Token>& tokens){
    size_t offset = 0;
    return parse_additive_expression(tokens,offset);
}

static Node* parse_additive_expression(const std::vector<Token>& tokens,size_t& offset){
    //tmp offset
    size_t t_offset = offset;
    
    //stack
    std::vector<Node*> mul_stack;
    std::vector<const Token*> ope_stack;
    
    //read first multicative expression
    Node* node = parse_multicative_expression(tokens,t_offset);
    if (node != nullptr){
        mul_stack.push_back(node);
    }else{
        return nullptr;
    }
    
    //read combinations of additive operation and multicative expression
    while (t_offset < tokens.size()){
        if (check_token("+",tokens,t_offset) ||
            check_token("-",tokens,t_offset))
        {
            ope_stack.push_back(&tokens[t_offset]);
            t_offset++;
            
            Node* node = parse_multicative_expression(tokens,t_offset);
            if (node != nullptr){
                mul_stack.push_back(node);
            }else{
                ope_stack.pop_back();
                t_offset--;
                break;
            }
        }else{
            break;
        }
    }
    
    //create node tree
    node = mul_stack[0];
    if (ope_stack.size() > 0){
        for (size_t i = 1;i < mul_stack.size();i++){
            node = new Node(node,mul_stack[i],ope_stack[i-1]);
        }
    }
    
    //success
    offset = t_offset;
    return node;
}

static Node* parse_multicative_expression(const std::vector<Token>& tokens,size_t& offset){
    //tmp offset
    size_t t_offset = offset;
    
    //stack
    std::vector<Node*> pri_stack;
    std::vector<const Token*> ope_stack;
    
    //read first primary expression
    Node* node = parse_primary_expression(tokens,t_offset);
    if (node != nullptr){
        pri_stack.push_back(node);
    }else{
        return nullptr;
    }
    
    //read combinations of multicative operation and primary expression
    while (t_offset < tokens.size()){
        if (check_token("*",tokens,t_offset) ||
            check_token("/",tokens,t_offset) ||
            check_token("%",tokens,t_offset))
        {
            ope_stack.push_back(&tokens[t_offset]);
            t_offset++;
            
            Node* node = parse_primary_expression(tokens,t_offset);
            if (node != nullptr){
                pri_stack.push_back(node);
            }else{
                ope_stack.pop_back();
                t_offset--;
                break;
            }
        }else{
            break;
        }
    }
    
    //create node tree
    node = pri_stack[0];
    if (ope_stack.size() > 0){
        for (size_t i = 1;i < pri_stack.size();i++){
            node = new Node(node,pri_stack[i],ope_stack[i-1]);
        }
    }
    
    //success
    offset = t_offset;
    return node;
}


static Node* parse_primary_expression(const std::vector<Token>& tokens,size_t& offset){
    //read integer or (additive expression)
    if (check_token(TOKEN_INTEGER,tokens,offset)){
        Node* node = new Node(nullptr,nullptr,&tokens[offset]);
        offset++;
        return node;
    }else{
        //tmp offset
        size_t t_offset = offset;
        
        //read token "("
        if (check_token("(",tokens,t_offset)){
            t_offset++;
        }else{
            return nullptr;
        }

        //read additive expression
        Node* node = parse_additive_expression(tokens,t_offset);
        if (node == nullptr){
            return nullptr;
        }

        //read token ")"
        if (check_token(")",tokens,t_offset)){
            t_offset++;
        }else{
            return nullptr;
        }
        
        //success
        offset = t_offset;
        return node;
    }
}


static bool check_token(const std::string& data,const std::vector<Token>& tokens,size_t offset){
    //check access range
    if (offset >= tokens.size()){
        return false;
    }

    //check token data
    if (tokens[offset].data == data){
        return true;
    }else{
        return false;
    }
}

static bool check_token(TokenType type,const std::vector<Token>& tokens,size_t offset){
    //check access range
    if (offset >= tokens.size()){
        return false;
    }

    //check token data
    if (tokens[offset].type == type){
        return true;
    }else{
        return false;
    }
}


static void generate(std::string& dst,const Node* node);
static void generate_expression(std::string& dst,const Node* node);

static void generate(std::string& dst,const Node* node){
    dst += (std::string)".globl _main" + "\n";
    dst += (std::string)"_main:" + "\n";
    dst += (std::string)"\t" + "pushq %rbp" +"\n";
    dst += (std::string)"\t" + "movq %rsp,%rbp" +"\n";
    
    generate_expression(dst,node);
    
    dst += (std::string)"\t" + "popq %rsi" + "\n";
    dst += (std::string)"\t" + "movq literal@GOTPCREL(%rip),%rdi" + "\n";
    dst += (std::string)"\t" + "callq _printf" + "\n";
    
    dst += (std::string)"\t" + "popq %rbp" +"\n";
    dst += (std::string)"\t" + "retq" +"\n";
    
    dst += (std::string)"literal:" + "\n";
    dst += (std::string)"\t" + ".asciz \"%lld\\n\"" + "\n";
}

static void generate_expression(std::string& dst,const Node* node){
    if (node->child[0] != nullptr && node->child[1] != nullptr){
        //generate code of child node
        generate_expression(dst,node->child[0]);
        generate_expression(dst,node->child[1]);
        
        //generate code of this node
        if (node->token == nullptr){
            std::cout << "failed to generate code" << "\n";
            std::cout << "node->token is nullptr" << "\n";
            std::terminate();
        }
        if (node->token->data == "+"){
            dst += (std::string)"\t" + "popq %rbx" + "\n";
            dst += (std::string)"\t" + "popq %rax" + "\n";
            dst += (std::string)"\t" + "addq %rbx,%rax" + "\n";
            dst += (std::string)"\t" + "pushq %rax" + "\n";
        }else if (node->token->data == "-"){
            dst += (std::string)"\t" + "popq %rbx" + "\n";
            dst += (std::string)"\t" + "popq %rax" + "\n";
            dst += (std::string)"\t" + "subq %rbx,%rax" + "\n";
            dst += (std::string)"\t" + "pushq %rax" + "\n";
        }else if (node->token->data == "*"){
            dst += (std::string)"\t" + "popq %rbx" + "\n";
            dst += (std::string)"\t" + "popq %rax" + "\n";
            dst += (std::string)"\t" + "mulq %rbx" + "\n";
            dst += (std::string)"\t" + "pushq %rax" + "\n";
        }else if (node->token->data == "/"){
            dst += (std::string)"\t" + "popq %rbx" + "\n";
            dst += (std::string)"\t" + "popq %rax" + "\n";
            dst += (std::string)"\t" + "movq $0,%rdx" + "\n";
            dst += (std::string)"\t" + "divq %rbx" + "\n";
            dst += (std::string)"\t" + "pushq %rax" + "\n";
        }else if (node->token->data == "%"){
            dst += (std::string)"\t" + "popq %rbx" + "\n";
            dst += (std::string)"\t" + "popq %rax" + "\n";
            dst += (std::string)"\t" + "movq $0,%rdx" + "\n";
            dst += (std::string)"\t" + "divq %rbx" + "\n";
            dst += (std::string)"\t" + "pushq %rdx" + "\n";
        }else{
            std::cout << "failed to generate code" << "\n";
            std::cout << "invalid token:" << node->token->data << "\n";
            std::terminate();
        }
    }else if (node->child[0] != nullptr){
        //generate code of child node
        generate_expression(dst,node->child[0]);
    }else{
        //generate code of this node
        if (node->token->type == TOKEN_INTEGER){
            dst += (std::string)"\t" + "pushq $" + node->token->data + "\n";
        }
    }
}

static void print_node(const Node* node,int level){
    if (node == nullptr){
        return;
    }
    
    std::string indent;
    for (int i = 0;i < level;i++){
        indent += ".";
    }
 
    std::cout << indent << "token data:";
    if (node->token != nullptr){
        std::cout << node->token->data;
    }else{
        std::cout << "null";
    }
    std::cout << "\n";
    
    print_node(node->child[0],level+1);
    print_node(node->child[1],level+1);
}

static void read_file(std::string& text,const std::string& path){
    //open file
    FILE* fp = fopen(path.c_str(),"r");
    if (fp == NULL){
        std::cout << "failed to open" << path << "\n";
        std::terminate();
    }

    //file size
    fseek(fp,0,SEEK_END);
    size_t size = ftell(fp);

    //allocate memory
    char* src = new char[size+1];

    //read file
    fseek(fp,0,SEEK_SET);
    fread(src,1,size,fp);
    src[size] = '\0';

    //copy file source
    text = src;

    //free memory
    delete[] src;

    //close file
    fclose(fp);
}

static void write_file(const std::string& text,const std::string& path){
    //open file
    FILE* fp = fopen(path.c_str(),"w");
    if (fp == NULL){
        std::cout << "failed to open" << path << "\n";
        std::terminate();
    }

    //write file
    fseek(fp,0,SEEK_SET);
    fwrite(text.c_str(),1,text.size(),fp);

    //close file
    fclose(fp);
}

int main(int argc,char* argv[]){
    //current directory
    char current_dir[1000];
    getcwd(current_dir,1000);
    
    //src file path
    if (argc < 2){
        std::cout << "input relative path of src file" << "\n";
        return -1;
    }
    std::string src_file_path = std::string(current_dir)+"/"+std::string(argv[1]);
    //std::cout<< "src_file_path:" << path << "\n";
    
    //read src file
    std::string src;
    read_file(src,src_file_path);
    
    //tokenize
    std::vector<Token> tokens;
    tokenize(tokens,src);
    
    //parse
    Node* root = parse(tokens);
    if (root == nullptr){
        std::cout << "failed to parse" << "\n";
        std::terminate();
    }
    
    //generate dst code
    std::string dst;
    generate(dst,root);
    
    //output dst file
    //erase .txt
    src_file_path.pop_back();
    src_file_path.pop_back();
    src_file_path.pop_back();
    src_file_path.pop_back();
    std::string dst_file_path = src_file_path+".s";
    write_file(dst,dst_file_path);
    
    //print_node(root,0);
    
    //delete node
    delete root;
}


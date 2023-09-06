#include <iostream>
#include <vector>
#include <memory>

class Process {
public:
        Process();
        Process(const std::string& script_name, const std::string& method, const std::string& argu_json);
        ~Process();
        static void push(const std::string& script_name, const std::string& method, const std::string& argu_json);
private:
        int data;
        std::string script_name;
        std::string method;
        std::string argu_json;
        std::string output;
        static std::vector<std::shared_ptr<Process>> gList;
};

// 类的静态变量需要在类外进行定义
std::vector<std::shared_ptr<Process>> Process::gList;

Process::Process()
{
        std::cout << "Process New" << std::endl;
}
Process::~Process() 
{
        std::cout << "Process Delete" << std::endl;
}

Process::Process(const std::string& script_name, const std::string& method, const std::string& argu_json)
: script_name(script_name), method(method), argu_json(argu_json)
{
        std::cout << "Process New" << std::endl;
}

void Process::push(const std::string& script_name, const std::string& method, const std::string& argu_json)
{
		// 这里使用emplace_back和push_back都可以
        gList.emplace_back(std::make_shared<Process>(script_name, method, argu_json));
        // gList.push_back(std::make_shared<Process>(script_name, method, argu_json));
        return;
}

int main()
{
        Process::push("hello.sh", "set", "helloworld");
        Process::push("hello.sh", "set", "helloworld");
        Process::push("hello.sh", "set", "helloworld");
        Process::push("hello.sh", "set", "helloworld");

        return 0;
}

#include <iostream>
#include <vector>
#include <memory>
#include <list>

class Process {
public:
        Process();
        ~Process();
private:
        std::string name;
};

std::list<std::weak_ptr<Process>> gList;

void clearWeakPtr()
{
    for (auto it = gList.begin(); it != gList.end();) {
        std::weak_ptr<Process> p = *it;
        if (p.expired()) {
            it = gList.erase(it);
        }else {
            it++;
        }
    }
}

// 使用智能智能指针的引用，不会增加引用次数
void addProcessToList(const std::shared_ptr<Process>& process)
{
    // 插入之前需要检查是否已经存在
    for (const auto& w: gList) {
        if (auto p = w.lock()) {
            if (p == process)
                return;
        }
    }
    gList.push_back(process);
}


Process::Process()
{
        std::cout << "Process New" << std::endl;
}
Process::~Process() 
{
        std::cout << "Process Delete" << std::endl;
}

int main()
{
    std::shared_ptr<Process> process = std::make_shared<Process>();
    std::cout << gList.size() << std::endl;
    addProcessToList(process);
    addProcessToList(process);
    addProcessToList(process);
    addProcessToList(process);
    addProcessToList(process);
    addProcessToList(process);
    process.reset();
    clearWeakPtr();
    std::cout << gList.size() << std::endl;
    
    return 0;
}

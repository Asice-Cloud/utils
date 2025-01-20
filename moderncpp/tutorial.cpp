#include<iostream>
#include<functional>

void download(std::string file,std::function<void(std::string)> callback){
    std::cout<<"Downloading file: "<<file<<std::endl;
    callback("Downloaded file: "+file);
}

int main(){
    download("tutorial.txt",[](std::string result){
        std::cout<<result<<std::endl;
    });
    return 0;
}
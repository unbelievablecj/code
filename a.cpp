#include<stdio.h>
#include<iostream>
#include<stack>
#include<string>
using namespace std;
int main(){
    //freopen("1.txt","r",stdin);
    stack<int> sta;
    string s;
    cin>>s;
    for(int i=0;i<s.length();i++){
        if(s[i]=='-'){
            if(!sta.empty()&&sta.top()==-1){
                sta.pop();
            }
            else{
                sta.push(-1);
            }
            cout<<sta.size()<<endl;
        }
        else{
            if(!sta.empty()&&sta.top()==1){
                sta.pop();
            }
            else{
                sta.push(1);
            }
        }
    }
    if(sta.empty()){
        cout<<"Yes";
    }
    else{
        cout<<"No";
    }
    return 0;
}

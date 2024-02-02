#include <iostream>
#include <vector>
#include <memory>
#include <thread>
#include <chrono>
#include <mutex>
#include <array>
#include <map>

struct Robot
{
    std::map<std::string,unsigned int> robo{{"Status",1},{"Level",1},{"LastHeatTime",0},{"LastTime",0}};
};

struct Soldier:Robot  //步兵
{
    Soldier(std::array<unsigned int,5> Info)
    {
        //是否存活，等级，机器人类型，血量上限，热量上限，血量，热量，所属队伍，机器人标识符
        robo["Type"]=0;
        robo["MaxBlood"]=150;
        robo["MaxHeat"]=100;
        robo["Blood"]=150;
        robo["Heat"]=0;
        robo["Team"]=Info[2];
        robo["Differ"]=Info[3];
        robo["LastTime"]=Info[0];
    }
};
struct Engin:Robot //工程
{
    Engin(std::array<unsigned int,5> Info)
    {
        robo["Type"]=1;
        robo["MaxBlood"]=300;
        robo["MaxHeat"]=0;
        robo["Blood"]=300;
        robo["Heat"]=0;
        robo["Team"]=Info[2];
        robo["Differ"]=Info[3];
        robo["LastTime"]=Info[0];
    }
};

struct Manage
{
    std::array<unsigned int,5> Info;
    std::vector<std::shared_ptr<Robot>> v;  //用于存放的容器

    Manage(){ v.reserve(20); }

    void print(){ std::cout<<"D "<<Info[2]<<' '<<Info[3]<<'\n'; }

    void StorageA()
    {
        int flag=0;
        if(v.size()==0)
        {
            if(Info[4]==0)
                v.push_back(std::make_shared<Soldier>(Info));
            else
                v.push_back(std::make_shared<Engin>(Info));
            return ;
        }

        for(auto &p:v)
        {
            if(p->robo["Status"]==0)//在被击毁的机器人中查询
            {
                if(p->robo["Type"]==Info[4] && p->robo["Team"]==Info[2] && p->robo["Differ"]==Info[3])
                {
                    p->robo["Status"]=1;
                    p->robo["Blood"]=p->robo["MaxBlood"];
                    p->robo["Heat"]=0;
                    p->robo["LastTime"]=Info[0];
                    flag=1;
                }
            }
            else
            {
                if(p->robo["Type"]==Info[4] && p->robo["Team"]==Info[2] && p->robo["Differ"]==Info[3])
                    flag=1;
            }
        }

        //未查找到，新建一个机器人信息
        if(flag==0)
        {
            if(Info[4]==0)
                v.push_back(std::make_shared<Soldier>(Info));
            else
                v.push_back(std::make_shared<Engin>(Info));
        }
    }

    void DistoryF()
    {
        for(auto &p:v)
        {
            if(p->robo["Status"]==1)
            {
                if(p->robo["Team"]==Info[2] && p->robo["Differ"]==Info[3])
                {
                    //计算当前血量
                    if(p->robo["Heat"]>p->robo["MaxHeat"])  //有过热量情况
                    {
                        int hurtVal;
                        if((p->robo["Heat"] - p->robo["MaxHeat"])>=(Info[0] - p->robo["LastHeatTime"]))
                            hurtVal=Info[0] - p->robo["LastHeatTime"];
                        else
                            hurtVal=p->robo["Heat"] - p->robo["MaxHeat"];
                        hurtVal+=Info[4]; //伤害结算

                        if(p->robo["Heat"] < (Info[0] - p->robo["LastHeatTime"]))
                            p->robo["Heat"]=0;
                        else
                            p->robo["Heat"]-=(Info[0] - p->robo["LastHeatTime"]);
                        p->robo["LastHeatTime"]=Info[0];  //热量结算


                        if(p->robo["Blood"]<hurtVal)
                            p->robo["Blood"]=0;
                        else
                            p->robo["Blood"]-=hurtVal;
                    }
                    else
                    {
                        if(p->robo["Blood"]<Info[4])
                            p->robo["Blood"]=0;
                        else
                            p->robo["Blood"]-=Info[4];
                    }

                    //判断是否被击毁
                    if(p->robo["Blood"]<=0)
                    {
                        p->robo["Status"]=0;
                        print();
                    }
                    else
                        p->robo["LastTime"]=Info[0];
                }
            }
        }
    }

    void Heat()
    {

        for(auto &p:v)
        {
            if(p->robo["Status"]==1)
            {
                if(p->robo["Team"]==Info[2] && p->robo["Differ"]==Info[3])
                {
                    if(p->robo["Type"]==0)
                    {
                        //保存当前热量
                        if(p->robo["Heat"]>0)
                        {
                            //保存因过热损失的血量
                            if(p->robo["Heat"]>p->robo["MaxHeat"])
                            {
                                int hurtVal;
                                if((p->robo["Heat"] - p->robo["MaxHeat"]) > (Info[0] - p->robo["LastHeatTime"]))
                                    hurtVal=Info[0] - p->robo["LastHeatTime"];
                                else
                                    hurtVal=p->robo["Heat"] - p->robo["MaxHeat"];
                                p->robo["Blood"]-=hurtVal;
                            }

                            else
                            {
                                if(p->robo["Heat"]>=(Info[0] - p->robo["LastHeatTime"]))
                                    p->robo["Heat"]=p->robo["Heat"]-(Info[0] - p->robo["LastHeatTime"])+Info[4];
                                else
                                    p->robo["Heat"]=Info[4];
                            }
                        }
                        else
                            p->robo["Heat"]=Info[4];

                        //计算是否直接过热量死亡
                        if(p->robo["Heat"] - p->robo["MaxHeat"] > p->robo["Blood"])
                        {
                            p->robo["Status"]=0;
                            print();
                        }

                        p->robo["LastHeatTime"]=Info[0];
                        p->robo["LastTime"]=Info[0];
                    }
                }
            }
        }
    }

    void Upgrade()
    {
        for(auto &p:v)
        {
            if(p->robo["Status"]==1)
            {
                if(p->robo["Team"]==Info[2] && p->robo["Differ"]==Info[3])
                {
                    if(p->robo["Type"]==0 && Info[4] > p->robo["Level"])
                    {
                        p->robo["Level"]=Info[4];
                        p->robo["MaxBlood"]+=(p->robo["Level"]-1)*50;
                        p->robo["MaxHeat"]=p->robo["Level"]*100;
                        p->robo["Blood"]=p->robo["MaxBlood"];
                        p->robo["Heat"]=p->robo["MaxHeat"];
                    }
                }
            }
        }
    }
};


int main()
{
    long long int dirNum;
    std::cin>>dirNum;

    Manage Pool;
    char tmp;
    int i,j;
    for(i=0;i<dirNum;i++)
    {
        for(j=0;j<5;j++)
        {
            if(j==1)
            {
                std::cin>>tmp;
                Pool.Info[1]=(int)tmp;
            }
            else
                std::cin>>Pool.Info[j];
        }

        switch(Pool.Info[1])
        {
            case 65:Pool.StorageA();break;
            case 70:Pool.DistoryF();break;
            case 72:Pool.Heat();break;
            case 85:Pool.Upgrade();break;
        }
    }
    return 0;
}

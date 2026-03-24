/*
 * @Descripttion: 
 * @version: 
 * @Author: Qiu Yijia
 * @Date: 2023-08-17 09:11:13
 * @LastEditors: Qiu Yijia
 * @LastEditTime: 2023-08-17 09:11:14
 */
#include "DataManger.hh"


DataManger*DataManger::instance=nullptr;

DataManger::DataManger(){
	SetDir();
	G4cout<<"*********Data Stored Directory is Set to: "<<outputdir.c_str()<<"*********"<<G4endl;
}

DataManger::~DataManger(){
	G4cout<<"*********DataManger Deleted*********** "<<G4endl;

}

DataManger* DataManger::GetInstance(){
    if (instance ==nullptr){
        instance = new DataManger();
    }
    return instance;
}

void DataManger::SetDir(){
	time_t t = time(nullptr);
	char ch[64] = {0};
	strftime(ch, sizeof(ch) - 1, "%Y%m%d%H%M%S", localtime(&t));
	G4String datatime=ch;
	G4String commend="mkdir ../OutPut/"+datatime;
	system (commend.c_str());
	outputdir="../OutPut/"+datatime;
}


/*
 * @Descripttion: 
 * @version: 
 * @Author: Qiu Yijia
 * @Date: 2023-08-17 09:09:57
 * @LastEditors: Qiu Yijia
 * @LastEditTime: 2023-08-17 09:09:58
 */
#ifndef DataManger_h
#define DataManger_h 1

#include "globals.hh"
#include <iostream>
#include <sstream>

class DataManger
{
public:

	static DataManger* GetInstance();
    virtual ~DataManger();
    G4String  GetDir(){return outputdir;};
    void SetDir();
    void AddRun(){RunIndex++;}
    G4int GetRun(){return RunIndex;}

    template<typename out_type, typename in_value>
    out_type convert(const in_value & t){
    	std::stringstream stream;
    	stream<<t;
    	out_type result;
    	stream>>result;
    	return result;
    }

private:
    DataManger();
    static DataManger* instance;
    G4String outputdir;
    G4int RunIndex=0;
};

#endif

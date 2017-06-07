//
//  main.cpp
//  OpenCV Example
//
//  Created by Jong Uk Kim on 2017. 5. 20..
//  Copyright © 2017년 Jong Uk Kim. All rights reserved.
//


#include "TimeTable.hpp"
#include "json/json.h"

void printScheduleToJson(schedule_list* schedules, const char* filename);

void printScheduleToJson(schedule_list* schedules, const char* filename)
{
    const char* week_day[] = {"월", "화", "수", "목", "금", "토", "일"};
    int day_i = 0;
    
    Json::Value root;
    
    
    if(schedules->size() == 0)
    {
        root["result"] = false;
        root["message"] = "Failed Processing Schedule.";
        goto PRINT_JSON;
    }
    else
    {
        root["result"] = true;
        root["message"] = "Process Parsing Schedule.";
    }
    
    std::for_each(schedules->begin(), schedules->end(), [&](std::vector<schedule> schedule_days)
                  {
                      
                      Json::Value schedule_day;
                      schedule_day["day"] = week_day[day_i];
                      
                      Json::Value schedule_time;
                      std::for_each(schedule_days.begin(), schedule_days.end(), [&](schedule schedule_data)
                                    {
                                        std::string str;
                                        
                                        str += std::to_string(schedule_data.start_time.hour) + ':';
                                        if(schedule_data.start_time.minute == 0)
                                        {
                                            str += "00";
                                        }
                                        else if(schedule_data.start_time.minute < 10)
                                        {
                                            str += '0';
                                            str += std::to_string(schedule_data.start_time.minute);
                                        }
                                        else
                                        {
                                            str += std::to_string(schedule_data.end_time.minute);
                                        }
                                        str += " ~ ";
                                        str += std::to_string(schedule_data.end_time.hour) + ':';
                                        if(schedule_data.end_time.minute == 0)
                                        {
                                            str += "00";
                                        }
                                        else if(schedule_data.end_time.minute < 10)
                                        {
                                            str += '0';
                                            str += std::to_string(schedule_data.end_time.minute);
                                        }
                                        else
                                        {
                                            str += std::to_string(schedule_data.end_time.minute);
                                        }
                                        
                                        
                                        schedule_time.append(str);
                                    });
                      
                      schedule_day["time"] = schedule_time;
                      
                      root["data"].append(schedule_day);
                      
                      day_i++;
                  });
    
PRINT_JSON:
    Json::StyledWriter writer;
    
    std::string str_name(filename);
    
    int i = 0;
    while(str_name[i] != '.')
        i++;
    
    str_name = str_name.substr(0, i);
    str_name += ".json";
    
    std::ofstream outputfile(str_name.c_str());
    
    outputfile << writer.write(root) << std::endl;
    
    outputfile.close();
    
    std::cout << "Print schedule to " << str_name << std::endl;
}


int main(int argc, const char * argv[]) {

    /////////LOAD IMAGE

    /////////Program name     |    File name    |    GAP   |   Start Time
    const char* image_name = argv[1];
    
    int image_gap_week = 0;
    int image_gap_time = 0;
    int start_time = -1;
    
    if(argc >= 3)
        image_gap_week = atoi(argv[2]);
    if(argc >= 4)
        image_gap_time = atoi(argv[3]);
    if(argc >= 5)
        start_time = atoi(argv[4]);
    
    cv::Mat full_image = cv::imread(image_name, CV_LOAD_IMAGE_COLOR);
    if(!full_image.data)
    {
        std::cout << "Error : Failed image file loading" << std::endl;
        return 0;
    }

    show(&full_image);

    ///////////SET DATA
    
    cv::Point cross_point = makeCrossPoint(&full_image);
    
    
#if DEBUG_PRINT
    std::cout << "Cross Point : " << cross_point.x << " , " << cross_point.y << std::endl;
#endif
    
    
    int width = full_image.cols;
    int height = full_image.rows;
    
    cv::Mat week_image = full_image.rowRange(image_gap_week, cross_point.y - image_gap_week).colRange(cross_point.x, width).clone();
    cv::Mat time_image = full_image.colRange(image_gap_time, cross_point.x - image_gap_time).rowRange(cross_point.y, height).clone();
    cv::Mat schedule_image = full_image.colRange(cross_point.x, width).rowRange(cross_point.y, height).clone();
    
#if DEBUG_PRINT
    cv::Mat schedule_image_2 = schedule_image.clone();
    cv::rectangle(full_image, cv::Point(0,0), cross_point, cv::Scalar(200), -1);
#endif
    
    show(&full_image);
    show(&week_image);
    week_data week = makeWeekData(&week_image);
    
#if DEBUG_PRINT
    std::cout << "요일 수 : " << week.week_num << std::endl << "요일 축 : ";
    for(int i=0; i< week.week_num; i++)
        std::cout << week.day_points[i] << " ,";
    std::cout << std::endl;
#endif
    
    show(&time_image);
    time_data time = makeTimeData(&time_image);
    
    if(start_time != -1)
        time.start_time = start_time;
    
#if DEBUG_PRINT
    std::cout << "시작 시간 : " << time.start_time << ", 한시간 길이 : " << time.hour << std::endl;
#endif
    
    
    
    
    //////////////FIND CONTOURS
    
    
    deleteGrayColor(&schedule_image);
    cv::Mat schedule_gray_image;
    cv::cvtColor(schedule_image, schedule_gray_image, CV_RGB2GRAY);
    cv::Mat schedule_binary_image;
    cv::threshold(schedule_gray_image, schedule_binary_image, 240, 255, CV_THRESH_BINARY_INV);
    
    contour_list contours;
    cv::findContours(schedule_binary_image, contours, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE);
    
    auto delete_contours = std::remove_if(contours.begin(), contours.end(), [](std::vector<cv::Point> contour)
                   {
                       if(cv::contourArea(contour) < 500)
                           return true;
                       else
                           return false;
                   });
    
    contours.erase(delete_contours, contours.end());
    
    
    
    ///////////MAKE SCHEDULE DATA
    
    schedule_value_list schedule_values(week.week_num);
    makeScheduleValue(&contours, &schedule_values, &week);
    
    schedule_list schedules(week.week_num);
    makeScheduleList(&schedule_values, &schedules, &time);
    
    
    
    const char* week_day[] = {"월", "화", "수", "목", "금", "토", "일"};
    
#if DEBUG_PRINT
    int day_i = 0;
    std::for_each(schedules.begin(), schedules.end(), [&](std::vector<schedule> schedule_data)
                  {
                      if(schedule_data.size() > 0)
                          std::cout << week_day[day_i] << std::endl;
                      day_i++;
                      
                      for(int i = 0; i < schedule_data.size(); i++)
                      {
                          schedule data = schedule_data[i];
                          if(data.name != NULL)
                              std::cout << data.name << " - ";
                          std::cout << data.start_time.hour << ":" << data.start_time.minute << " ~ ";
                          std::cout << data.end_time.hour << ":" << data.end_time.minute << std::endl;
                      }
                  });
#endif
    
    
    printScheduleToJson(&schedules, image_name);
    
    //////////////DRAW DATA
    
    
#if DEBUG_PRINT
    show(&schedule_image);
    cv::drawContours(schedule_image, contours, -1, cv::Scalar(200), 3);
    show(&schedule_image);
    
    drawData(&schedule_image, week, time);
    
    show(&schedule_image);
    
    
    
    //cv::drawContours(schedule_image_2, contours, -1, cv::Scalar(200), 3);
    
    //cv::namedWindow("Contoured Schedule");
    //cv::imshow("Contoured Schedule", schedule_image_2);
    //cv::waitKey(0);
    
    //cv::destroyWindow("Contoured Schedule");
#endif
    return 0;
}




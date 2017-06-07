//
//  TimeTable.hpp
//  OpenCV Example
//
//  Created by Jong Uk Kim on 2017. 5. 21..
//  Copyright © 2017년 Jong Uk Kim. All rights reserved.
//

#ifndef TimeTable_hpp
#define TimeTable_hpp


//#define DEBUG_PRINT 1


#include <iostream>
#include <fstream>
#include <algorithm>
#include <vector>
#include <map>

#include <opencv2/opencv.hpp>

#include <tesseract/baseapi.h>
#include <leptonica/allheaders.h>


void show(cv::Mat* image);


/////////////////////SET DATA

typedef struct Week_Data
{
    int week_num;
    std::vector<int> day_points;
}week_data;

typedef struct Time_Data
{
    int start_time;
    double hour;
    double five_minutes;
}time_data;

cv::Point makeCrossPoint(cv::Mat* full_image);

week_data makeWeekData(cv::Mat* week_image);
time_data makeTimeData(cv::Mat* time_image);

bool checkGrayColor(cv::Vec3b pixel_color);
void deleteGrayColor(cv::Mat* image);



/////////////////////MAKE SCHEDULE DATA

typedef struct Schedule_Time
{
    int hour;
    int minute;
}schedule_time;

typedef struct Schedule
{
    schedule_time start_time;
    schedule_time end_time;
    const char* name;
}schedule;

typedef std::vector<std::vector<int> > schedule_value_list;
typedef std::vector<std::vector<schedule> > schedule_list;
typedef std::vector<std::vector<cv::Point> > contour_list;

void addTimeValue(schedule_value_list* schedule_values, cv::Point point_1, cv::Point point_2, week_data* week);
void makeScheduleValue(contour_list* contours, schedule_value_list* schedule_values, week_data* week);

void tessClass(tesseract::TessBaseAPI* tess, cv::Mat* class_image, const char* class_name);
void makeScheduleTime(int line_1, int line_2, schedule* schedule_ptr, time_data* time);
void makeScheduleList(schedule_value_list* schedule_values, schedule_list* schedules, time_data* time);

void tessScheduleName(cv::Mat* schedule_image, schedule_value_list* schedule_values, schedule_list* schedules, week_data* week);


/////////////////////DRAW DATA

void drawData(cv::Mat* image, week_data week, time_data time);







#endif /* TimeTable_hpp */






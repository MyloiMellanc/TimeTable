//
//  TimeTable.cpp
//  OpenCV Example
//
//  Created by Jong Uk Kim on 2017. 5. 21..
//  Copyright © 2017년 Jong Uk Kim. All rights reserved.
//

#include "TimeTable.hpp"


#define LOOP_MIN 5
#define LOOP_MAX 15

#define GRAY_CUT 245
#define GRAY_GAP 5

void show(cv::Mat* image)
{
#if DEBUG_PRINT
    int width = image->cols;
    int height = image->rows;
    if(width * height < 100000)
    {
        cv::Mat sized_image;
        cv::resize(*image, sized_image, cv::Size(image->cols * 2, image->rows) * 2);
        cv::namedWindow("demo");
        cv::imshow("demo", sized_image);
        cv::waitKey(0);
        cv::destroyWindow("demo");

        sized_image.release();
    }
    else
    {
        cv::namedWindow("demo");
        cv::imshow("demo", *image);
        cv::waitKey(0);
        cv::destroyWindow("demo");
    }
#endif
}


cv::Point makeCrossPoint(cv::Mat* full_image)
{
    // X Position
    std::vector<int> line_x;
    line_x.clear();
    
    
    for(int y = LOOP_MIN; y < LOOP_MAX; y++)
    {
        int x = LOOP_MIN;
        cv::Vec3b color_begin = full_image->at<cv::Vec3b>(y, x);
        int gray_begin = (color_begin[0] + color_begin[1] + color_begin[2]) / 3;
        
        while(x < full_image->cols)
        {
            
            cv::Vec3b color = full_image->at<cv::Vec3b>(y, x);
    
            int gray_end = (color[0] + color[1] + color[2]) / 3;
            if(gray_begin - gray_end >= GRAY_GAP)
            {
                line_x.push_back(x);
                break;
            }
            
            x++;
        }
    }
    
    // Y Position
    
    std::vector<int> line_y;
    line_y.clear();

    
    for(int x = LOOP_MIN; x < LOOP_MAX; x++)
    {
        int y = LOOP_MIN;
        cv::Vec3b color_begin = full_image->at<cv::Vec3b>(y, x);
        int gray_begin = (color_begin[0] + color_begin[1] + color_begin[2]) / 3;
        
        while(y < full_image->rows)
        {
            cv::Vec3b color = full_image->at<cv::Vec3b>(y, x);
            
            int gray_end = (color[0] + color[1] + color[2]) / 3;
            if(gray_begin - gray_end >= GRAY_GAP)
            {
                line_y.push_back(y);
                break;
            }
            
            y++;
        }
    }
    
    std::map<int, int> map_x, map_y;
    map_x.clear();
    map_y.clear();
    
    std::for_each(line_x.begin(), line_x.end(), [&](int x)
                  {
                      auto iter = map_x.find(x);
                      if(iter != map_x.end())
                      {
                          iter->second++;
                      }
                      else
                      {
                          map_x.insert(std::pair<int, int>(x, 1));
                      }
                  });

    std::for_each(line_y.begin(), line_y.end(), [&](int y)
                  {
                      auto iter = map_y.find(y);
                      if(iter != map_y.end())
                      {
                          iter->second++;
                      }
                      else
                      {
                          map_y.insert(std::pair<int, int>(y, 1));
                      }
                  });
    
    
    int key_x = 0;
    int value_x = 0;
    
    std::for_each(map_x.begin(), map_x.end(), [&](std::pair<int, int> obj)
                  {
                      if(value_x < obj.second)
                      {
                          key_x = obj.first;
                          value_x = obj.second;
                      }
                  });
    
    int key_y = 0;
    int value_y = 0;
    
    std::for_each(map_y.begin(), map_y.end(), [&](std::pair<int, int> obj)
                  {
                      if(value_y < obj.second)
                      {
                          key_y = obj.first;
                          value_y = obj.second;
                      }
                  });
    

    cv::Point cross_point(key_x, key_y);
    
    return cross_point;
}



#define CANNY_MIN 250
#define CANNY_MAX 400


week_data makeWeekData(cv::Mat* week_image)
{
    week_data week;
    week.day_points.clear();
    
    
    int width = week_image->cols;
    int height = week_image->rows;
    
    int block_range = width / 13;
    
    
    cv::Mat canny_image;
    cv::Canny(*week_image, canny_image, CANNY_MIN, CANNY_MAX);
    
    show(&canny_image);

    for(int x = LOOP_MIN; x < width - LOOP_MIN; x++)
    {
        for(int y = LOOP_MIN; y < height - LOOP_MIN; y++)
        {
            if(canny_image.at<uchar>(y,x) > 200)
            {
                week.day_points.push_back(x);
                x += block_range;
                break;
            }
        }
    }
    
    week.week_num = (int)(week.day_points.size());
    
    
    return week;
}

#define BLOCK_NOIZE 2
#define BLOCK_RANGE 30

#define PIXEL_DIFF 1
#define BLOCK_SIZE_MULTI 3

#define BASIC_START_TIME 9

time_data makeTimeData(cv::Mat* time_image)
{
    time_data time;
    
    int block_num = 0;
    int width = time_image->cols;
    int height = time_image->rows;
    
    cv::Mat canny_image;
    
    cv::Canny(*time_image, canny_image, CANNY_MIN, CANNY_MAX);
    
    show(&canny_image);
    
    for(int y = BLOCK_NOIZE; y < height - BLOCK_NOIZE; y++)
    {
        for(int x = BLOCK_NOIZE; x < width - BLOCK_NOIZE; x++)
        {
            if(canny_image.at<uchar>(y, x) > 200)
            {
                block_num++;
                
                y += BLOCK_RANGE;
            }
        }
    }
    
    
#if DEBUG_PRINT
    std::cout << "시간블럭 개수 : " << block_num << std::endl;
#endif
    
    double block_length = (double)height / (double)block_num;
    
    time.hour = block_length;
    
    time.five_minutes = time.hour / 12.0;
    
    
    cv::Mat start_block_default = time_image->rowRange(0, (int)block_length).colRange(0, width).clone();
    
    cv::Mat start_block;
    
    cv::resize(start_block_default, start_block, cv::Size(start_block_default.cols * BLOCK_SIZE_MULTI, start_block_default.rows * BLOCK_SIZE_MULTI));
    
    
    
    tesseract::TessBaseAPI* tess = new tesseract::TessBaseAPI();
    if(tess->Init(NULL, "kor")) //returns zero on success.
    {
       //exit(1);
        time.start_time = BASIC_START_TIME;
        return time;
    }
    
    
    tess->SetImage((uchar*)start_block.data, start_block.cols, start_block.rows, start_block.channels(), start_block.step1());
    
    //tess->SetVariable("classify_bln_numeric_mode", "1");
    tess->SetVariable("tessedit_char_whitelist", "0123456789");
    tess->Recognize(0);
    const char* out = tess->GetUTF8Text();
    
    
#if DEBUG_PRINT
    printf("Tesseract Result : %s\n", out);
#endif
    
    time.start_time = atoi(out);
    
    if((time.start_time > 23) || (time.start_time < 0))
        time.start_time = BASIC_START_TIME;
    
    return time;
}



///////////////////////////////////////////////////////////////
#define GRAY_DIFF 5
///////////////////////////////////////////////////////////////

bool checkGrayColor(cv::Vec3b pixel_color)
{
    
    int color_gray = (pixel_color[0] + pixel_color[1] + pixel_color[2]) / 3;
    
    if(abs(pixel_color[0] - color_gray) <= GRAY_DIFF)
        if(abs(pixel_color[1] - color_gray) <= GRAY_DIFF) 
            if(abs(pixel_color[2] - color_gray) <= GRAY_DIFF)
                return true;
    
    return false;

}

void deleteGrayColor(cv::Mat* image)
{
    //if Image is gray, don't work.
    if(image->channels() == 1)
        return;
    
    int width = image->cols;
    int height = image->rows;
    
    for(int x = 0; x < width; x++)
    {
        for(int y = 0; y < height; y++)
        {
            cv::Vec3b pixel_color = image->at<cv::Vec3b>(y, x);
            if(checkGrayColor(pixel_color) == true)
            {
                image->at<cv::Vec3b>(y, x)[0] = 255;
                image->at<cv::Vec3b>(y, x)[1] = 255;
                image->at<cv::Vec3b>(y, x)[2] = 255;
            }
        }
    }
}


void addTimeValue(schedule_value_list* schedule_values, cv::Point point_1, cv::Point point_2, week_data* week)
{
    if(week->week_num == 0)
        return;

    for(int i = 0; i < week->week_num; i++)
    {
        int weekday = week->day_points[i];
        if(point_1.x > point_2.x)
            std::swap(point_1, point_2);
        
        if((point_1.x <= weekday) && (weekday < point_2.x))
        {
            int point_average = (point_1.y + point_2.y) / 2;
            schedule_values->at(i).push_back(point_average);
        }
    }
}


void makeScheduleValue(contour_list* contours, schedule_value_list* schedule_values, week_data* week)
{
    std::for_each(contours->begin(), contours->end(), [&](std::vector<cv::Point> contour)
                  {
                      for(int p1 = 0; p1 < contour.size(); p1++)
                      {
                          int p2 = (p1 + 1) % contour.size();
                          addTimeValue(schedule_values, contour[p1], contour[p2], week);
                      }
                  });
    
    for(int i = 0; i < schedule_values->size(); i++)
    {
        std::sort(schedule_values->at(i).begin(), schedule_values->at(i).end(), std::greater<int>());
    }
}


void tessClass(tesseract::TessBaseAPI* tess, cv::Mat* class_image, const char** class_name)
{
    cv::Mat resized_class_image;
    cv::resize(*class_image, resized_class_image, cv::Size(class_image->cols * BLOCK_SIZE_MULTI, class_image->rows * BLOCK_SIZE_MULTI));
    
    cv::namedWindow("test");
    cv::imshow("test", resized_class_image);
    cv::waitKey(0);
    cv::destroyWindow("test");
    
    tess->SetImage((uchar*)resized_class_image.data, resized_class_image.cols, resized_class_image.rows, resized_class_image.channels(), resized_class_image.step1());
    
    tess->Recognize(0);
    const char* out = tess->GetUTF8Text();
    std::cout << out << std::endl;
    //*class_name = tess->GetUTF8Text();
    *class_name = out;

}


void makeScheduleTime(int line_1, int line_2, schedule* schedule_ptr, time_data* time)
{
    int five_minutes = line_1 / time->five_minutes;
    
    double rest = (double)line_1 - ((double)five_minutes * time->five_minutes);
        
    if(rest > (time->five_minutes / 2.0))
        five_minutes++;
    
    
    schedule_ptr->start_time.hour = (five_minutes / 12) + time->start_time;
    schedule_ptr->start_time.minute = (five_minutes % 12) * 5;
    
    five_minutes = line_2 / time->five_minutes;
    
    rest = (double)line_2 - ((double)five_minutes * time->five_minutes);
    if(rest > (time->five_minutes / 2.0))
        five_minutes++;
    
    
    schedule_ptr->end_time.hour = (five_minutes / 12) + time->start_time;
    schedule_ptr->end_time.minute = (five_minutes % 12) * 5;
    
    schedule_ptr->name = NULL;
}


void makeScheduleList(schedule_value_list* schedule_values, schedule_list* schedules, time_data* time)
{
    for(int i = 0; i < schedule_values->size(); i++)
    {
        int value_num = schedule_values->at(i).size() - 1;
        while(value_num >= 1)
        {
            int value_1 = schedule_values->at(i)[value_num];
            value_num--;
            
            int value_2 = schedule_values->at(i)[value_num];
            value_num--;
            
            schedule schedule_data;
            makeScheduleTime(value_1, value_2, &schedule_data, time);
            
            schedules->at(i).push_back(schedule_data);
        }
    }
}


void tessScheduleName(cv::Mat* schedule_image, schedule_value_list* schedule_values, schedule_list* schedules, week_data* week)
{
    tesseract::TessBaseAPI* tess = new tesseract::TessBaseAPI();
    if(tess->Init(NULL, "kor"))
        return;
    
    int day_width = schedule_image->cols / week->week_num;
    for(int i = 0; i < week->week_num; i++)
    {
        int width_left = day_width * i;
        int width_right = day_width * (i + 1);
        
        int class_num = 0;
        int value_num = schedule_values->at(i).size() - 1;
        while(value_num >= 1)
        {
            int value_down = schedule_values->at(i)[value_num];
            value_num--;
            
            int value_up = schedule_values->at(i)[value_num];
            value_num--;
            
            cv::Mat class_block = schedule_image->colRange(width_left, width_right).rowRange(value_down, value_up).clone();
        
            tessClass(tess, &class_block, &schedules->at(i)[class_num].name);
        }
        
    }
}


void drawData(cv::Mat* image, week_data week, time_data time)
{
    std::for_each(week.day_points.begin(), week.day_points.end(), [&](int weekday)
                  {
                      cv::Point point_up(weekday, 0);
                      cv::Point point_down(weekday, image->rows);
                      cv::line(*image, point_up, point_down, cv::Scalar(50), 1);
                  });
    
    for(double i = time.hour; i < (float)image->rows;)
    {
        cv::Point point_left(0, i);
        cv::Point point_right(image->cols, i);
        cv::line(*image, point_left, point_right, cv::Scalar(50), 1);
        
        i += time.hour;
    }
    
    
}



/**
 ******************************************************************************
 * @file    ControllerScreenViews.h
 * @authors mat
 * @date    10 March 2015
 ******************************************************************************
  Copyright (c) 2015 Spark Labs, Inc.  All rights reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation, either
  version 3 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, see <http://www.gnu.org/licenses/>.
 ******************************************************************************
 */

#ifndef CONTROLLERSCREENVIEWS_H
#define	CONTROLLERSCREENVIEWS_H

extern "C" {
#include "d4d.h"
}

#include "controller_screen.h"
#include "TempControl.h"
#include "TemperatureFormats.h"

bool set_background_color(const D4D_OBJECT* pThis, D4D_COLOR bg);



class ControllerStateView
{        
    const D4D_OBJECT** backgrounds;
    
public:
    
    static const unsigned NUM_OBJECTS = 2;

    /**
     * Defines the widgets that show the background state
     * @param backgrounds
     */    
    ControllerStateView(const D4D_OBJECT* backgrounds[NUM_OBJECTS]) {
        this->backgrounds = backgrounds;
    }
    
    void update(D4D_COLOR bg, const char* text)
    {
        D4D_SetText(backgrounds[0], text);
        for (unsigned i=0; i<NUM_OBJECTS; i++) {
            set_background_color(backgrounds[i], bg);
        }
    }
};

class ControllerStatePresenter
{
    static D4D_COLOR state_color[];
    static const char* state_name[];
    ControllerStateView& view_;
    
    D4D_COLOR colorForState(states state)
    {
        return state_color[state];
    }
    
    const char* nameForState(states state) 
    {
        return state_name[state];
    }

public:    
    
    ControllerStatePresenter(ControllerStateView& view)
    : view_(view)
    {        
    }
    
    void setState(states state)
    {
        view_.update(colorForState(state), nameForState(state));
    }
    
};

class TemperatureProcessView
{
    const D4D_OBJECT**    objects;
    
public:

    /**
     * The first object is the current temperature.
     * The second object is the set point.
     * @param objects
     */
    TemperatureProcessView(const D4D_OBJECT* objects[]) {
        this->objects = objects;
    }
    
    void setBgColor(D4D_COLOR col)
    {
        set_background_color(objects[0], col);
        set_background_color(objects[1], col);
        set_background_color(objects[2], col);
    }
    
    void update(const char* currentTemp, const char* setpoint)
    {
        D4D_SetText(objects[0], currentTemp);
        if (setpoint) {
            D4D_SetText(objects[1], setpoint ? setpoint : "");            
        }        
    }
};

/**
 * Presents both a set point and the current temperature.
 */
class TemperatureProcessPresenter
{
    TemperatureProcessView& view_;    
    D4D_COLOR bg_col;
public:

    TemperatureProcessPresenter(TemperatureProcessView& view, D4D_COLOR col) :
        view_(view), bg_col(col) {}
    
    void update(temperature current, temperature setpoint, bool has_setpoint=true)
    {
        char current_str[MAX_TEMP_LEN];
        char setpoint_str[MAX_TEMP_LEN];
        
        tempToString(current_str, current, 1, MAX_TEMP_LEN);
        tempToString(setpoint_str, setpoint, 1, MAX_TEMP_LEN);
        view_.setBgColor(bg_col);
        view_.update(current_str, has_setpoint ? setpoint_str : NULL);
    }          
};

class ControllerModeView
{
    const D4D_OBJECT* obj;
    
public:

    ControllerModeView(const D4D_OBJECT* obj)
    {
        this->obj = obj;
    }
    
    void update(const char* mode, D4D_COLOR color)
    {
        D4D_SetText(obj, mode);
        set_background_color(obj, color);
    }
};


class ControllerModePresenter
{
    ControllerModeView& view_;
    
    static const char modes[5];
    static const char* names[5];
    static D4D_COLOR colors[5];
    
    unsigned modeToIndex(control_mode_t mode) 
    {
        for (unsigned int i=0; i<arraySize(modes); i++) 
        {
            if (modes[i]==mode)
                return i;
        }        
        return 3;   // OFF
    }
    
    const char* nameForMode(control_mode_t mode) {
        return names[modeToIndex(mode)];
    }
    
    D4D_COLOR colorForMode(control_mode_t mode) {
        return colors[modeToIndex(mode)];
    }
    
public:

    ControllerModePresenter(ControllerModeView& view)
            : view_(view) {}
            
    void update(control_mode_t mode)
    {
        view_.update(nameForMode(mode), colorForMode(mode));
    }
};


class ControllerTimeView
{
    const D4D_OBJECT* obj;
public:

    ControllerTimeView(const D4D_OBJECT* obj)
    {
        this->obj = obj;
    }
    
    void update(const char* time) 
    {
        D4D_SetText(obj, time);
    }
};

uint16_t fetch_time(states state)
{
    tcduration_t time = INT_MIN;
    tcduration_t sinceIdleTime = tempControl.timeSinceIdle();
    if(state==IDLE){
        time = min(tempControl.timeSinceCooling(), tempControl.timeSinceHeating());
    }
    else if(state==COOLING || state==HEATING){
        time = sinceIdleTime;
    }
    else if(state==COOLING_MIN_TIME){
        time = MIN_COOL_ON_TIME-sinceIdleTime;
    }	
    else if(state==HEATING_MIN_TIME){
        time = MIN_HEAT_ON_TIME-sinceIdleTime;
    }
    else if(state == WAITING_TO_COOL || state == WAITING_TO_HEAT){
        time = tempControl.getWaitTime();
    }

    return time;
}

class ControllerTimePresenter
{
    ControllerTimeView& view_;
    
public:

    ControllerTimePresenter(ControllerTimeView& view)
        : view_(view) {}
            
        void update() {
            char time_str[MAX_TIME_LEN];
            tcduration_t time = tempControl.getWaitTime();
            if (time==INT_MIN)
                time_str[0] = 0;
            else
                sprintf(time_str, "%d:%02d:%02d", uint16_t(time/3600), uint16_t((time/60)%60), uint16_t(time%60));
            view_.update(time_str);
        }
};

const char ControllerModePresenter::modes[5] = {
    MODE_FRIDGE_CONSTANT,
    MODE_BEER_CONSTANT,
    MODE_BEER_PROFILE,
    MODE_OFF,
    MODE_TEST    
};

const char* ControllerModePresenter::names[5] = {
    "FRIDGE",
    "BEER",
    "PROFILE",
    "OFF",
    "TEST"
};

D4D_COLOR ControllerModePresenter::colors[5] = {
    MODE_FRIDGE_COLOR,
    MODE_BEER_COLOR,
    MODE_PROFILE_COLOR,
    MODE_OFF_COLOR,
    MODE_TEST_COLOR
};

#endif	/* CONTROLLERSCREENVIEWS_H */

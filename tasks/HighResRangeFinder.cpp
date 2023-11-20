/* Generated from orogen/lib/orogen/templates/tasks/Task.cpp */

#include "HighResRangeFinder.hpp"
#include <mars/interfaces/sim/SensorManagerInterface.h>

using namespace mars;

HighResRangeFinder::HighResRangeFinder(std::string const& name)
    : HighResRangeFinderBase(name)
{
}

HighResRangeFinder::HighResRangeFinder(std::string const& name, RTT::ExecutionEngine* engine)
    : HighResRangeFinderBase(name, engine)
{
}

HighResRangeFinder::~HighResRangeFinder()
{
    std::vector<Camera*>::iterator it = cameras.begin();
    for(; it != cameras.end(); ++it) {
        delete *it;
    }
    cameras.clear();
}

bool HighResRangeFinder::addCamera(::std::string const & name, double orientation)
{
    long sensor_id = control->sensors->getSensorID( name );
    if( !sensor_id ){
	    RTT::log(RTT::Error) << "There is no camera by the name of " << name << " in the scene" << RTT::endlog();
        return false;
    }
    
    mars::sim::CameraSensor* cam_sensor = dynamic_cast<mars::sim::CameraSensor *>(control->sensors->getSimSensor(sensor_id));
    if( !cam_sensor){
        RTT::log(RTT::Error) << "CameraPlugin: Given sensor name is not a camera" << RTT::endlog();
        return false;
    }
    
    HighResRangeFinderCamera cam = { name, orientation };
    addedCameras.push_back(cam);

    //using !cameras.empty() as a proxy for the task being started. In a started task,
    //there will always be at least one camera.
    if(!cameras.empty()) {
        Camera* camera = new Camera(sensor_id, cam_sensor, orientation);
        camera->name = name;
        calcCamParameters(camera);
        cameras.push_back(camera);
    }
    return true;
}
    
    
void HighResRangeFinder::calcCamParameters(Camera* camera) {
    
    int width = camera->cam_sensor_info.width;
    int height = camera->cam_sensor_info.height;
    double opening_width = camera->cam_sensor_info.opening_width;
    double opening_height = camera->cam_sensor_info.opening_height;
    
    // Calculate starting pixel (bottom left within the image plane)
    camera->pixel_per_rad_horizontal = width / ((opening_width / 180.0) * M_PI);
    camera->pixel_per_rad_vertical = height / ((opening_height / 180.0) * M_PI);
    
    // Sets the borders.    
    camera->lower_pixel = _lower_limit.get() * camera->pixel_per_rad_vertical + height/2.0;
    camera->upper_pixel = _upper_limit.get() * camera->pixel_per_rad_vertical + height/2.0;
    camera->left_pixel = _left_limit.get() * camera->pixel_per_rad_horizontal + width/2.0;
    camera->right_pixel = _right_limit.get() * camera->pixel_per_rad_horizontal + width/2.0;
    
    if(camera->lower_pixel < 0) {
        RTT::log(RTT::Warning) << "Lower limit exceeds the image plane, will be scaled down by " << 
                std::fabs(camera->lower_pixel) << " pixel" << RTT::endlog();
        camera->lower_pixel = 0;
    }
    if(camera->upper_pixel > height) {
        RTT::log(RTT::Warning) << "Upper limit exceeds the image plane, will be scaled down by " << 
                camera->upper_pixel - height << " pixel" << RTT::endlog();
        camera->upper_pixel = height;
    }
    if(camera->left_pixel < 0) {
        RTT::log(RTT::Warning) << "Left limit exceeds the image plane, will be scaled down by " << 
                std::fabs(camera->left_pixel) << " pixel" << RTT::endlog();
        camera->left_pixel = 0;
    }
    if(camera->right_pixel > width) {
        RTT::log(RTT::Warning) << "Right limit exceeds the image plane, will be scaled down by " << 
                camera->right_pixel - width << " pixel" << RTT::endlog();
        camera->right_pixel = width;
    }
   
    camera->v_steps = _resolution_vertical.get() * camera->pixel_per_rad_vertical;
    camera->h_steps = _resolution_horizontal.get() * camera->pixel_per_rad_horizontal;
    
    RTT::log(RTT::Info) << "Camera " << camera->sensor_id << " (" << camera->name << ") added" << RTT::endlog();
    RTT::log(RTT::Info) << "opening_width " << opening_width << ", opening_height " << opening_height << RTT::endlog();
    RTT::log(RTT::Info) << "Horizontal: Every " << camera->h_steps << " pixel " << " will be used from " <<
            camera->left_pixel << " to " << camera->right_pixel << RTT::endlog();
    RTT::log(RTT::Info) << "Vertical: Every " << camera->v_steps << " pixel " << " will be used from " <<
            camera->lower_pixel << " to " << camera->upper_pixel << RTT::endlog();
}


/// The following lines are template definitions for the various state machine
// hooks defined by Orocos::RTT. See HighResRangeFinder.hpp for more detailed
// documentation about them.

bool HighResRangeFinder::configureHook()
{
    if (! HighResRangeFinderBase::configureHook())
        return false;
        
    return true;
}
bool HighResRangeFinder::startHook()
{
    if (! HighResRangeFinderBase::startHook())
        return false;
        
    // Adds this camera to the list of cameras.
    Camera* camera = new Camera(sensor_id, this->camera, 0.0);
    calcCamParameters(camera);
    cameras.push_back(camera);

    for(auto &c : addedCameras) {
        long sensor_id = control->sensors->getSensorID( c.name );
        if( !sensor_id ){
            RTT::log(RTT::Error) << "There is no camera by the name of " << c.name << " in the scene" << RTT::endlog();
        }

        mars::sim::CameraSensor* cam_sensor = dynamic_cast<mars::sim::CameraSensor *>(control->sensors->getSimSensor(sensor_id));
        if( !cam_sensor){
            RTT::log(RTT::Error) << "CameraPlugin: Given sensor name is not a camera" << RTT::endlog();
        }

        Camera* camera = new Camera(sensor_id, cam_sensor, c.orientation);
        camera->name = c.name;
        calcCamParameters(camera);
        cameras.push_back(camera);
    }

    for(auto &c : _extra_cameras.get()) {
        long sensor_id = control->sensors->getSensorID( c.name );
        if( !sensor_id ){
            RTT::log(RTT::Error) << "There is no camera by the name of " << c.name << " in the scene" << RTT::endlog();
        }

        mars::sim::CameraSensor* cam_sensor = dynamic_cast<mars::sim::CameraSensor *>(control->sensors->getSimSensor(sensor_id));
        if( !cam_sensor){
            RTT::log(RTT::Error) << "CameraPlugin: Given sensor name is not a camera" << RTT::endlog();
        }

        Camera* camera = new Camera(sensor_id, cam_sensor, c.orientation);
        camera->name = c.name;
        calcCamParameters(camera);
        cameras.push_back(camera);
    }

    return true;
}

void HighResRangeFinder::updateHook()
{
    HighResRangeFinderBase::updateHook();
}
void HighResRangeFinder::errorHook()
{
    HighResRangeFinderBase::errorHook();
}
void HighResRangeFinder::stopHook()
{
    HighResRangeFinderBase::stopHook();

    std::vector<Camera*>::iterator it = cameras.begin();
    for(; it != cameras.end(); ++it) {
        delete *it;
    }
    cameras.clear();
}
void HighResRangeFinder::cleanupHook()
{
    HighResRangeFinderBase::cleanupHook();
}

void HighResRangeFinder::getData()
{	
    Eigen::Matrix<double, 3, 1> scene_p;
    base::samples::Pointcloud pointcloud;
    pointcloud.time = getTime();
    size_t x_t = 0, y_t = 0;
    int counter = 0;
    std::vector<Camera*>::iterator it = cameras.begin();
    for(; it < cameras.end(); ++it) {
        counter = 0;
        // Request image and store it within the base distance image.
        (*it)->camera_sensor->getDepthImage((*it)->image->data);
        base::samples::DistanceImage* image = (*it)->image;
        
        for(double y = (*it)->lower_pixel; y < (*it)->upper_pixel; y += (*it)->v_steps) {
            for(double x = (*it)->left_pixel; x < (*it)->right_pixel; x += (*it)->h_steps) {
                // Pixel contains a distance value between min and max
                // and lies within the image plane.
                x_t = (size_t) x;
                y_t = (size_t) y;
                if((*it)->image->data[x_t+y_t*(*it)->image->width] >= _minimum_distance.get() &&
                        (*it)->image->data[x_t+y_t*(*it)->image->width] <= _maximum_distance.get() &&
                        (*it)->image->getScenePoint<double>( (size_t) x, (size_t) y, scene_p )) {
                    // Rotate camera around the y axis.
                    scene_p = (*it)->orientation * scene_p;
                    // Transforms to robot frame (x: front, z: up) and adds to the pointcloud.
                    scene_p = base::Vector3d(scene_p[2], -scene_p[0], -scene_p[1]);
                    pointcloud.points.push_back(scene_p);
                    counter++;
                }
            }
        }
        RTT::log(RTT::Info) << counter << " points have been added by camera " << (*it)->name <<
                " ID "<< (*it)->sensor_id << RTT::endlog();
    }
    _pointcloud.write(pointcloud);
}

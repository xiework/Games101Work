//
// Created by LEI XU on 5/16/19.
//

#ifndef RAYTRACING_INTERSECTION_H
#define RAYTRACING_INTERSECTION_H
#include "Vector.hpp"
#include "Material.hpp"
class Object;
class Sphere;

struct Intersection
{
    Intersection(){
        happened=false;
        coords=Vector3f();
        normal=Vector3f();
        distance= std::numeric_limits<double>::max();
        obj =nullptr;
        m=nullptr;
    }
    bool happened;
    //交点在世界坐标系中的位置（三维坐标）
    Vector3f coords;
    //通常用于存储纹理坐标（UV/st）或其他插值坐标
    Vector3f tcoords;
    Vector3f normal;
    //交点处材质的自发光颜色（辐射亮度 L_e），若该物体是发光体，则这里存储其发光强度/颜色。
    Vector3f emit;
    //从射线原点到交点的距离（参数 t 或 length）
    double distance;
    //指向发生相交的物体实例的指针
    Object* obj;
    //指向交点处材质的指针
    Material* m;
};
#endif //RAYTRACING_INTERSECTION_H

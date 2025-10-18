//
// Created by Göksu Güvendiren on 2019-05-14.
//

#include "Scene.hpp"


void Scene::buildBVH() {
    printf(" - Generating BVH...\n\n");
    this->bvh = new BVHAccel(objects, 1, BVHAccel::SplitMethod::NAIVE);
}

Intersection Scene::intersect(const Ray &ray) const
{
    return this->bvh->Intersect(ray);
}

void Scene::sampleLight(Intersection &pos, float &pdf) const
{
    float emit_area_sum = 0;
    for (uint32_t k = 0; k < objects.size(); ++k) {
        if (objects[k]->hasEmit()){
            emit_area_sum += objects[k]->getArea();
        }
    }
    float p = get_random_float() * emit_area_sum;
    emit_area_sum = 0;
    for (uint32_t k = 0; k < objects.size(); ++k) {
        if (objects[k]->hasEmit()){
            emit_area_sum += objects[k]->getArea();
            if (p <= emit_area_sum){
                objects[k]->Sample(pos, pdf);
                break;
            }
        }
    }
}

bool Scene::trace(
        const Ray &ray,
        const std::vector<Object*> &objects,
        float &tNear, uint32_t &index, Object **hitObject)
{
    *hitObject = nullptr;
    for (uint32_t k = 0; k < objects.size(); ++k) {
        float tNearK = kInfinity;
        uint32_t indexK;
        Vector2f uvK;
        if (objects[k]->intersect(ray, tNearK, indexK) && tNearK < tNear) {
            *hitObject = objects[k];
            tNear = tNearK;
            index = indexK;
        }
    }


    return (*hitObject != nullptr);
}

// Implementation of Path Tracing
Vector3f Scene::castRay(const Ray &ray, int depth) const
{
    // TO DO Implement Path Tracing Algorithm here
    //先发射一条射线
    Intersection p_inter = intersect(ray);
    //判断是否有射中物体
    if (!p_inter.happened) {
        return Vector3f(0.0f);
    }
    //如果射中了发光体，直接返回发光体的颜色
    if (p_inter.m->hasEmission()) {
        return p_inter.m->getEmission();
    }

    float EPLISON = 0.0001;
    Vector3f L_dir = Vector3f(0.0f);

    //计算直接光照
    /*
        1.采样一个光源位置
        2.构造一个射线，从交点指向光源位置
        3.判断该射线是否被遮挡
        4.若未被遮挡，计算光源贡献的光照
    */
    Intersection light_inter;
    float pdf_light = 0.0f;
    //1.采样一个光源位置
    sampleLight(light_inter, pdf_light);
    //Get x, ws, NN, emit from inter
    Vector3f p = p_inter.coords; // 交点位置
    Vector3f x = light_inter.coords; // 光源位置
    Vector3f ws = (x - p).normalized(); // 光源方向
    float ws_distance = (x - p).norm(); // 光源距离
    Vector3f N = p_inter.normal.normalized(); // 交点法线
    Vector3f NN = light_inter.normal.normalized(); // 光源法线
    Vector3f emit = light_inter.emit; // 光源发光颜色

    //2.构造一个射线，从交点指向光源位置
    Ray ws_ray(p,ws);
    //3.判断该射线是否被遮挡
    Intersection ws_ray_inter = intersect(ws_ray);

    if (ws_ray_inter.distance - ws_distance > -EPLISON) //发射的光线与光源到交点的距离基本一致，说明未被遮挡
    {
        //4.若未被遮挡，计算光源贡献的光照
        L_dir = emit * 
                p_inter.m->eval(ray.direction,ws,N) * 
                dotProduct(ws,N) * 
                dotProduct(-ws,NN) / 
                std::pow(ws_distance,2) / 
                pdf_light;
    }

    // 俄罗斯轮盘赌，决定是否计算间接光照
    if (get_random_float() > RussianRoulette) {
        return L_dir;
    }
    
    //计算间接光照
    Vector3f L_indir = Vector3f(0.0f);
    //确定出射方向
    Vector3f wi_dir = p_inter.m->sample(ray.direction, N).normalized();
    //构造新的射线
    Ray wi_ray(p_inter.coords, wi_dir);
    //计算新的射线与场景的交点
    Intersection wi_inter = intersect(wi_ray);
    //判断交点是否合法
    if (wi_inter.happened && !wi_inter.m->hasEmission()) // 合法且不是光源
    {
        //计算pdf
        float pdf_indir = p_inter.m->pdf(ray.direction, wi_dir, N);
        //计算间接光照贡献
        L_indir = castRay(wi_ray,depth + 1) *
                    p_inter.m->eval(ray.direction, wi_dir, N) *
                    dotProduct(wi_dir, N) /
                    pdf_indir /
                    RussianRoulette;
    }

    return L_dir + L_indir;
}
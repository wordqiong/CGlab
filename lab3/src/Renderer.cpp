#include"Ray.h"
#include"Scene.h"
#include"Renderer.h"
#include"hittable.h"
#include"Material.h"
#include"Camera.h"
#include<limits>
#include<fstream>



Renderer::Renderer()
	:width(600), height(600),image(nullptr)
{

}

Renderer::~Renderer() {
	if (image)
		delete image;
}

color Renderer::trace(const Ray& ray, const shared_ptr<Scene>& scene,int depth) {
	const float reflect_atten = 0.7f;
	const float refract_atten = 0.3f;
	const float ka = 0.05f;
	if (depth >= 5) {
		return color(0.0f);
	}

	hit_record rec;
	constexpr float infinity = std::numeric_limits<float>::infinity();

	bool isHit = scene->hit(ray, 0.001f, infinity, rec);
	if (!isHit) {
		return background;
	}

	vec3 finalColor = vec3(0.0f);
	//TODO: implement ray tracing and shading algorithm for 3 type of material.
	//确认材质
	vec3 light_pos = vec3(0.0f, 4.0f, 0.0f);
	vec3 light_color = vec3(1.0f, 1.0f, 1.0f);
	#define max(a,b) ((a)>(b)?(a):(b))
	switch (rec.mat_ptr->type)
	{
		//代码解释过长报错了，详细看报告吧
	case MatType::DIFFUSE:{
		vec3 shadow_direction = light_pos - rec.p;
		Ray shadow_ray(rec.p, shadow_direction);

		color shadow_intensity(0.0f);
		hit_record shadow_record;
		bool intersects = scene->hit(shadow_ray, 0.001f, infinity, shadow_record);
		bool isHitShadow = shadow_record.t > 0.775f;
		if (intersects && isHitShadow) {
			// ambient light
			shadow_intensity = ka * background;
			// diff light
			float diffuse = max(dot(normalize(shadow_direction), normalize(rec.normal)), 0.0f);
			shadow_intensity += 0.3f * diffuse * light_color;
			// specular light
			vec3 view_direction = normalize(ray.dir);
			vec3 reflect_direction = normalize(reflect(shadow_direction, rec.normal));
			float specular = pow(max(dot(view_direction, reflect_direction), 0.0), 15);
			shadow_intensity += specular * light_color;
			shadow_intensity *= rec.mat_ptr->albedo;
		}
		else if (intersects) {
			// shadow
			shadow_intensity = ka * background * rec.mat_ptr->albedo;
		}
		else {
			//background
			shadow_intensity = ka * background;
		}

		// reflect ray
		color reflection_intensity;
		vec3 reflection_direction = reflect(ray.dir, rec.normal);
		reflection_intensity = reflect_atten * trace(Ray(rec.p, reflection_direction), scene, depth + 1);

		finalColor = reflect_atten * shadow_intensity;
		break;
	}
	case MatType::LIGHT: {
		finalColor = rec.mat_ptr->albedo;
		break;
	}
	case MatType::GLASS: {
			//代码解释过长报错了，详细看报告吧
		float ref_idx = 1.7f;
		double refraction_ratio = rec.front_face ? (1.0 / ref_idx) : ref_idx;

		double cos_theta = fmin(dot(normalize(-ray.dir), normalize(rec.normal)), 1.0);
		double sin_theta = sqrt(1.0 - cos_theta * cos_theta);

		bool cannot_refract = refraction_ratio * sin_theta > 1.0;
		vec3 direction;

		vec3 refracted_ray_perpendicular_component = float(refraction_ratio) * (normalize(-ray.dir) + float(cos_theta) * normalize(rec.normal));
		vec3 refracted_ray_parallel_component = -sqrtf(fabs(1.0 - refracted_ray_perpendicular_component.length() * refracted_ray_perpendicular_component.length())) * normalize(rec.normal);
		vec3 refracted_ray_direction = refracted_ray_parallel_component + refracted_ray_perpendicular_component;
		vec3 reflected_ray_direction = reflect(ray.dir, rec.normal);
		color reflect_color = reflect_atten * trace(Ray(rec.p, reflected_ray_direction), scene, depth + 1);
		color refract_color(0.0f);
		if (cannot_refract)
			reflect_color /= reflect_atten;
		else
			refract_color = refract_atten * trace(Ray(rec.p, refracted_ray_direction), scene, depth + 1);
		finalColor = reflect_color + refract_color;

	}
	default:
		break;
	}
	//end of TODO
	return finalColor;
}

#include<iostream>
void Renderer::render(const shared_ptr<Scene>& scene) {
	auto& camera = scene->camera;
	int h = camera->height;
	int w = camera->width;

	image = new int[3 * w * h];

	for (int j = h-1; j >= 0; j--) {
		if (j % 20 == 0) {
			std::cout << (h-1-j) * 1.0f / (h - 1) << "%\n";
		}
		for (int i = 0; i < w; i++) {
			vec3 color(0, 0, 0);
			auto u = float(i) / (w - 1);
			auto v = float(j) / (h - 1);
			Ray r = camera->get_ray(u, v);
			//std::cout << r.dir.x << ' ' << r.dir.y << ' ' << r.dir.z << '\n';
			color = trace(r,scene,0);

			int pos = j * w + i;
			//write_color(color, pos);
			color = glm::clamp(glm::sqrt(color),0.0f,1.0f);
			image[3 * pos] = 255 * color.x;
			image[3 * pos + 1] = 255 * color.y;
			image[3 * pos + 2] = 255 * color.z;
		}
	}
}

void Renderer::writePic(const string& filename, const shared_ptr<Scene>&scene) {
	std::cout << "Writing to file: " << filename << '\n';
	auto& camera = scene->camera;

	std::ofstream f(filename);
	int h = camera->height;
	int w = camera->width;
	f << "P3\n" << camera->width << ' ' << camera->height << "\n255\n";

	for (int j = h - 1; j >= 0; j--) {
		for (int i = 0; i < w; i++) {
			int index = j * w + i;
			f << image[3 * index] << ' ' << image[3 * index + 1]
				<< ' ' << image[3 * index + 2] << '\n';
		}
	}
}

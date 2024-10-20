//
//  main.cpp
//  SpringMass
//
//  Created by Hyun Joon Shin on 2021/05/09.
//

#include <iostream>
#include <JGL/JGL_Window.hpp>
#include "AnimView.hpp"
#include <glm/gtx/quaternion.hpp>

using namespace glm;

struct Particle {
	// 현재 위치와 속도로 define한 Particle
	// newton 방정식은 2차 미분 방정식이므로 현재 상태로 x를 바로 계산하는 게 어려움(적분 2번 해야되어서)
	// v를 계산하는 것으로 바꾸어 1차로 바꿈 (a에서 v 계산) (v -> x 역시 1차), 따라서 1차 2번 하면 됨 0605 47
	vec3 x;
	vec3 v;
	vec3 f; // accumulate all the force
	float m;
	Particle( float mass, const vec3& position, const vec3& velocity=vec3(0) )
		: x( position ), v( velocity ), m( mass ) {}
	void clearForce() {
		f = vec3(0);
	}
	void add( const vec3& force ) {
		f+=force;
	}
	void update( float deltaT ) {
		v += f / m * deltaT;
		x += v * deltaT;
	}
	void draw() {
		drawSphere( x, 1 );
	}
};

struct Spring {
	Particle& a;
	Particle& b;
	float restLength;
	float k = 4.8f;
	float kd = 0.0f;
	Spring( Particle& x, Particle& y ) : a(x), b(y), restLength( length(x.x-y.x)) {
	}
	void addForce() {
		vec3 dx = a.x - b.x;
		vec3 dx_ = normalize(dx);
		vec3 dv = a.v - b.v;
		vec3 f = (k * ( length(dx) - restLength) + kd * dot(dv, dx_)) * dx_;
		a.add(-f);
		b.add(f);
	}
	void draw() {
		drawCylinder( a.x, b.x, 0.4, glm::vec4(0,1,.4,1) );
	}
};

struct Plane {
	vec3 N;
	vec3 p;
	float alpha = 0.6;
	Plane( const vec3& position, const vec3& normal ): N(normal), p(position){}
	void draw() {
		drawQuad(p,N,{1000,1000},vec4(0,0,1,1));
	}
	void resolveCollision( Particle& particle ) {
		float d = dot(N, particle.x - p);
		const float eps = 0.0001;
		if (d < eps) {
			float v = dot(N, particle.v);
			if (v < -eps) {
				vec3 vN = v * N;
				vec3 vT = particle.v - vN;
				particle.v = vT - alpha * vN;
			} else if (v < eps) {
				vec3 vN = v * N;
				vec3 vT = particle.v - vN;
				particle.v = vT;
			}
			particle.x += -d * N;
		}
	}
};

struct Sphere {
	vec3 N;
	vec3 p;
	float radius;
	float alpha = 0.6;
	Sphere(const vec3& position,const float& r) : p(position), radius(r) {}
	void draw() {
		drawSphere(p, radius, vec4(1, 1, 0, alpha));
	}
	void resolveCollision(Particle& particle) {
		N = normalize(particle.x - p);
		float d = dot(N, particle.x - p) - radius;
		const float eps = 0.01;
		if (d < eps) {
			float v = dot(N, particle.v);
			if (v < -eps) {
				vec3 vN = v * N;
				vec3 vT = particle.v - vN;
				particle.v = vT - alpha * vN;
			}
			else if (v < eps) {
				vec3 vN = v * N;
				vec3 vT = particle.v - vN;
				particle.v = vT;
			}
			particle.x += -d * N;
		}
	}
};

float randf() {
	return rand()/(float)RAND_MAX;
}
bool fix0 = true, fix1 = true;

void keyFunc(int key) {
	if( key == '1' )
		fix0=!fix0;
	if( key == '2' )
		fix1=!fix1;
}

const vec3 G ( 0, -980.f, 0 );
const float k_drag = 0.01f;
std::vector<Particle> particles;
std::vector<Spring> springs;
Plane flooring( {0,0,0}, {0,1,0} );
Sphere sphere({0,30,-5}, 30.f);
const int count = 20;

void init() {
	particles.clear();
	float randomness = 0.0;
	for (int y = 0; y < count; ++y) {
		for (int x = 0; x < count; ++x) {
			particles.emplace_back(0.001, vec3(x * 40.f/count - 18.f + randf() * randomness, 
				y * 40.f/count + 70 + randf() * randomness, randf() * 0.001));
		}
	}
	/*for (auto& p : particles)
		p.x = rotate(quat(sqrt(3) / 2, 0, 0, 0.5), p.x - vec3(10, 40, 0)) + vec3(10, 40, 0);*/
	for (int y = 0; y < count - 1; ++y) {
		for (int x = 0; x < count; ++x) {
			springs.emplace_back(particles[y * count + x], particles[(y + 1) * count + x]);
		}
	}
	for (int y = 0; y < count; ++y) {
		for (int x = 0; x < count - 1; ++x) {
			springs.emplace_back(particles[y * count + x], particles[y * count + x + 1] );
		}
	}
	for (int y = 0; y < count - 1; ++y) {
		for (int x = 0; x < count - 1; ++x) {
			springs.emplace_back(particles[y * count + x], particles[(y + 1) * count + x + 1]);
		}
	}
	for (int y = 0; y < count - 1; ++y) {
		for (int x = 0; x < count - 1; ++x) {
			springs.emplace_back(particles[(y+1) * count + x], particles[y * count + x + 1]);
		}
	}
	//springs.clear();
}
void frame( float dt ) {
	const int steps = 100;

	for( int i = 0; i < steps; i++ ) {
		vec3 p0 = particles[(count - 1) * count].x;
		vec3 p1 = particles[(count) * count - 1].x;
		for (auto& p : particles) p.clearForce();
		for (auto& s : springs) s.addForce();
		for (auto& p : particles) p.add(p.m * G); // Gravity
		for (auto& p : particles) p.add(-k_drag * p.v); // Viscous drag
		for (auto& p : particles) p.update(dt / steps);
		for (auto& p : particles) flooring.resolveCollision(p);
		for (auto& p : particles) sphere.resolveCollision(p);
		if (fix0) {
			particles[(count - 1) * count].x = p0;
			particles[(count - 1) * count].v = vec3(0, 0, 0);
		}
		if (fix1) {
			particles[count * count - 1].x = p1;
			particles[count * count - 1].v = vec3(0, 0, 0); 
		}
	}
	
}

void render() {
	for( auto& p : particles ) p.draw();
	for( auto& s : springs )   s.draw();
	flooring.draw();
	sphere.draw();
}

int main(int argc, const char * argv[]) {
	JGL::Window* window = new JGL::Window(800,600,"simulation");
	window->alignment(JGL::ALIGN_ALL);
	AnimView* animView = new AnimView(0,0,800,600);
	animView->renderFunction = render;
	animView->frameFunction = frame;
	animView->initFunction = init;
	animView->keyFunction = keyFunc;
	init();
	window->show();
	JGL::_JGL::run();
	return 0;
}

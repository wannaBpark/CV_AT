//
//  AnimView.hpp
//  SpringMass
//
//  Created by Hyun Joon Shin on 2021/05/12.
//

#ifndef AnimView_h
#define AnimView_h

#include "ModelView.hpp"

struct AnimView: ModelView {
	float lastT = 0;
	float _progress = 0;
	bool _animating = false;
	
	std::function<void()> initFunction=[](){};
	std::function<void(float)> frameFunction = [](float){};
	std::function<void(int)> keyFunction = [](int){};
	
	AnimView(float x, float y, float w, float h, const std::string& name="")
	: ModelView(x,y,w,h,name){}
	
	bool handle(int e) override {
		if( e == JGL::EVENT_KEYDOWN ) {
			keyFunction(JGL::_JGL::eventKey());
			if( JGL::_JGL::eventKey() == ' ' ) {
				_animating = !_animating;
				if( _animating ) {
					lastT = glfwGetTime();
					animate();
				}
				return true;
			}
			else if( JGL::_JGL::eventKey() == '0' ) {
				_animating = false;
				_progress = 0;
				initFunction();
				redraw();
				return true;
			}
		}
		return ModelView::handle( e );
	}
	void stopAnimation() {
		_animating = false;
	}
	void init() {
		_progress = 0;
		initFunction();
		_animating = true;
	}
	float progress() const { return _progress; }
	virtual void drawContents(NVGcontext* vg, const glm::rect&r, int a ) override {
		if( _animating ) {
			float t = glfwGetTime();
			float dt = t-lastT;
			_progress += dt;
			frameFunction(dt);
			animate();
			lastT = t;
		}
	}
};

#endif /* AnimView_h */

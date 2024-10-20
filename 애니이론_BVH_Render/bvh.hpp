#ifndef __BVH_HPP__
#define __BVH_HPP__

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <string>
#include <iostream>
#include <vector>
#include <glm/gtx/quaternion.hpp>
#include "GLTools.hpp"

const float OFFSET_SCALE = 5.f;

inline glm::vec3 rotate(const glm::quat& q, const glm::vec3& v) {
	glm::quat tmp = q * glm::quat(0, v) * inverse(q);
	return glm::vec3(tmp.x, tmp.y, tmp.z);
}

struct Link {
	enum class CHANNEL_TYPE {
		X_POSITION,
		Y_POSITION,
		Z_POSITION,
		X_ROTATION,
		Y_ROTATION,
		Z_ROTATION,
	};

	glm::vec3 offset;
	std::string name;
	std::vector<CHANNEL_TYPE> channelTypes;
	std::vector<Link*> children;

	glm::vec3 tr;
	glm::quat ro = glm::quat(1, 0, 0, 0);

	Link* parent = nullptr;
	~Link() {
		for (int i = 0; i < children.size(); i++)
			delete children[i];
		children.clear();
	}
	void addChild(Link* l) {
		children.push_back(l);
	}
	void print(std::ostream& os, int t) {
		for (int i = 0; i < t; i++) os << " ";
		os << name << std::endl;
		for (auto child : children)
			child->print(os, t + 1);
	}
	static Link* readJoint(std::istream& is, Link* parent) {
		Link* link = new Link();
		std::string tmp;
		int nChannels;
		is >> link->name;
		is >> tmp; // {
		is >> tmp; // OFFSET
		is >> link->offset.x >> link->offset.y >> link->offset.z;
		link->offset *= OFFSET_SCALE;
		is >> tmp; // CHANNELS
		is >> nChannels;
		for (int i = 0; i < nChannels; i++) {
			is >> tmp;
			if (tmp.compare("Xposition") == 0) link->channelTypes.push_back(CHANNEL_TYPE::X_POSITION);
			else if (tmp.compare("Yposition") == 0) link->channelTypes.push_back(CHANNEL_TYPE::Y_POSITION);
			else if (tmp.compare("Zposition") == 0) link->channelTypes.push_back(CHANNEL_TYPE::Z_POSITION);
			else if (tmp.compare("Xrotation") == 0) link->channelTypes.push_back(CHANNEL_TYPE::X_ROTATION);
			else if (tmp.compare("Yrotation") == 0) link->channelTypes.push_back(CHANNEL_TYPE::Y_ROTATION);
			else if (tmp.compare("Zrotation") == 0) link->channelTypes.push_back(CHANNEL_TYPE::Z_ROTATION);
		}
		while (true) {
			is >> tmp; // JOINT, End, or }
			if (tmp.compare("JOINT") == 0) link->addChild(readJoint(is, link));
			else if (tmp.compare("End") == 0) link->addChild(readEndSite(is, link));
			else if (tmp.compare("}") == 0) break;
		}
		return link;
	}
	static Link* readEndSite(std::istream& is, Link* parent) {
		Link* link = new Link();
		std::string tmp;
		is >> tmp; // Site
		is >> link->name;
		is >> tmp; // {
		is >> tmp; // OFFSET
		is >> link->offset.x >> link->offset.y >> link->offset.z;
		link->offset *= OFFSET_SCALE;
		is >> tmp; // }
		return link;
	}
	void draw(const glm::vec3& pp, const glm::quat& pq) {
		glm::quat q = pq * ro;
		glm::vec3 p = rotate(q, offset) + pp + tr;
		glm::mat4 m = translate(p);
		drawCylinder(p, pp, 1, glm::vec4(1, 0.3, 0, 1));
		drawSphere(p, 2, glm::vec4(1, 1, 0, 1));
		for (auto child : children)
			child->draw(p, q);
	}
};


struct Bone {
	enum class CHANNEL_TYPE {
		X_POSITION,
		Y_POSITION,
		Z_POSITION,
		X_ROTATION,
		Y_ROTATION,
		Z_ROTATION,
	};

	glm::vec3 offset;
	std::string name;
	std::vector<CHANNEL_TYPE> channelTypes;
	glm::vec3 tr = glm::vec3(0, 0, 0);
	glm::quat ro = glm::quat(1, 0, 0, 0);
	int parent = -1;
	int dataOffset = 0;
	glm::vec3 gp;
	glm::quat gq;
};

struct Body {
	std::vector<Bone> bones;
	std::string s;
	std::vector<float> motions;

	int m_totalChannels = 0;
	int m_NFrames;
	float m_FrameRate;
	void readEndSite(std::istream& is, int parent) {
		bones.push_back(Bone());
		Bone& bone = bones.back();
		bone.parent = parent;
		std::string tmp;
		is >> tmp; // Site
		is >> bone.name; std::cout << bone.name << std::endl;
		is >> tmp; // {
		is >> tmp; // OFFSET
		is >> bone.offset.x >> bone.offset.y >> bone.offset.z;
		bone.offset *= OFFSET_SCALE;
		is >> tmp; // }
	}

	void readBVH(const std::string& fn) {
		std::ifstream is(fn);
		std::stack<int> parent;
		std::string tmp;

		is >> tmp; // HIERARCHY
		int dataIndex = 0;
		while (bones.size() == 0 || !parent.empty()) {

			is >> tmp; // JOINT, End, or }

			if (tmp.compare("JOINT") == 0 || tmp.compare("ROOT") == 0) {

				bones.push_back(Bone());
				Bone& bone = bones.back();
				bone.parent = parent.empty() ? -1 : parent.top();
				bone.dataOffset = dataIndex;
				int nChannels;
				is >> bone.name; std::cout << bone.name << std::endl;
				is >> tmp; // {
				is >> tmp; // OFFSET
				is >> bone.offset.x >> bone.offset.y >> bone.offset.z;
				bone.offset *= OFFSET_SCALE;
				is >> tmp; // CHANNELS
				is >> nChannels;

				m_totalChannels += nChannels;
				for (int i = 0; i < nChannels; i++) {
					is >> tmp;
					if (tmp.compare("Xposition") == 0)      bone.channelTypes.push_back(Bone::CHANNEL_TYPE::X_POSITION);
					else if (tmp.compare("Yposition") == 0) bone.channelTypes.push_back(Bone::CHANNEL_TYPE::Y_POSITION);
					else if (tmp.compare("Zposition") == 0) bone.channelTypes.push_back(Bone::CHANNEL_TYPE::Z_POSITION);
					else if (tmp.compare("Xrotation") == 0) bone.channelTypes.push_back(Bone::CHANNEL_TYPE::X_ROTATION);
					else if (tmp.compare("Yrotation") == 0) bone.channelTypes.push_back(Bone::CHANNEL_TYPE::Y_ROTATION);
					else if (tmp.compare("Zrotation") == 0) bone.channelTypes.push_back(Bone::CHANNEL_TYPE::Z_ROTATION);
				}
				dataIndex += nChannels;
				parent.push(bones.size() - 1);
			}
			else if (tmp.compare("End") == 0) readEndSite(is, parent.top());
			else if (tmp.compare("}") == 0)
				parent.pop();
		}

		readFrames(is, 0);

		is.close();
	}

	void readFrames(std::istream& is, int parent)
	{
		int i = 0;
		is >> s; // MOTION
		is >> s >> m_NFrames;
		std::cout << s << " " << m_NFrames << std::endl;
		is >> s >> s >> m_FrameRate;
		std::cout << s <<" " << m_FrameRate << std::endl;
		std::cout << bones.size() << std::endl;
		while (i++ < m_NFrames * m_totalChannels) {
			float tmp;
			is >> tmp;
			motions.push_back(tmp);
		}
		std::cout << motions.size() << std::endl;
	}


	void assignMotion(int curFrame)
	{
		size_t curOffset = curFrame* m_totalChannels; // 0 : 0    1 : 75
		glm::vec3 eulerAngle(0.f);
		for (auto& bone : bones) {
			// dataOffset : current Bone의 데이터 시작 위치
			// curOffset : 현재 프레임 기준 motions 데이터 시작 위치
			size_t start = curOffset + bone.dataOffset;
			size_t sz = bone.channelTypes.size();
			bool isRot = false;
			for (auto i = 0; i < sz; ++i) {
				// 3일 때는 0 1 2
				// 6일 때는 0 1 2 3 4 5
				auto& channel = bone.channelTypes[i];
				auto& curMotion = motions[start + i];
				switch (channel) {
				case Bone::CHANNEL_TYPE::X_POSITION:
					bone.tr.x = curMotion * OFFSET_SCALE;
					break;
				case Bone::CHANNEL_TYPE::Y_POSITION:
					bone.tr.y = curMotion * OFFSET_SCALE;
					break;
				case Bone::CHANNEL_TYPE::Z_POSITION:
					bone.tr.z = curMotion * OFFSET_SCALE;
					break;
				case Bone::CHANNEL_TYPE::X_ROTATION:
					isRot = true;
					eulerAngle.x = glm::radians(curMotion);
					break;
				case Bone::CHANNEL_TYPE::Y_ROTATION:
					isRot = true;
					eulerAngle.y = glm::radians(curMotion);
					break;
				case Bone::CHANNEL_TYPE::Z_ROTATION:
					isRot = true;
					eulerAngle.z = glm::radians(curMotion);
					break;
				default:
					break;
				}
			}
			if (isRot) {
				float c1 = cosf(eulerAngle.x / 2);
				float c2 = cosf(eulerAngle.y / 2);
				float c3 = cosf(eulerAngle.z / 2);
				float s1 = sinf(eulerAngle.x / 2);
				float s2 = sinf(eulerAngle.y / 2);
				float s3 = sinf(eulerAngle.z / 2);

				bone.ro.w = c1 * c2 * c3 - s1 * s2 * s3;
				bone.ro.x = s1 * c2 * c3 - c1 * s2 * s3;
				bone.ro.y = c1 * s2 * c3 + s1 * c2 * s3;
				bone.ro.z = c1 * c2 * s3 + s1 * s2 * c3;

				eulerAngle = glm::vec3(0.f);
			}
		}
	}


	void update() {
		for (auto& b : bones) {
			if (b.parent >= 0) {
				b.gq = bones[b.parent].gq * b.ro;
				b.gp = rotate(bones[b.parent].gq, b.offset) + b.tr + bones[b.parent].gp;
			}
			else {
				b.gq = b.ro;
				b.gp = rotate(b.gq, b.offset) + b.tr;
			}
		}
	}
	void draw() {
		update();
		for (auto& b : bones) {
			if (b.parent >= 0)
				drawCylinder(b.gp, bones[b.parent].gp, 1, glm::vec4(1, 0, 0, 1));
		}
	}
	int getNFrames() const {
		return m_NFrames;
	}
	float getFrameRate() const {
		return m_FrameRate;
	}
};



#endif

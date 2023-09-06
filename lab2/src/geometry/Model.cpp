#include<glad/glad.h>
#include<GLFW/glfw3.h>
#include"Model.h"
#include"half-edge.h"
#include"../utils/Shader.h"
#include"Mesh.h"
#include<random>

template <class T>
static int statistic_cnt(std::vector<T> statical_arr) {
	int res = 0;
	for (const auto& element : statical_arr) {
		if (element->valid)
			++res;
	}
	return res;
}
inline float calIndices(Vertex* a,Vertex* b)
{
	if (a == NULL || b == NULL)
	{
		return -1;
	}
	float x_d, y_d, z_d;
	x_d = abs(a->position.x - b->position.x);
	y_d = abs(a->position.y - b->position.y);
	z_d = abs(a->position.z - b->position.z);
	float res;
	res =sqrt( x_d * x_d + y_d * y_d + z_d * z_d);
	return res;
}
int Model::findMinIndice()
{
	//一条边 判断可不可行
	std::vector<bool>judge(this->halfedges.size(),false);
	float min = FLT_MAX;
	int id = 0;
	for (int i = 0; i < this->halfedges.size(); i++)
	{
		if (this->halfedges[i]->valid == false||judge[i]==true)//已经被折叠了 or 已经被判断过了
		{
			//if (this->halfedges[i]->valid == false)
			//{
			//	int a;
			//	a = 1;
			//	std::cin >> a;
			//}
			continue;
		}
		judge[i] = true;//被访问过了
		if (this->halfedges[i]->opposite != NULL)
			judge[this->halfedges[i]->opposite->id - 1] = true;
		float temp=calIndices(this->halfedges[i]->incident_vertex, this->halfedges[i]->next->incident_vertex);
		if (temp < min)
		{
			id = this->halfedges[i]->id-1;
			min = temp;
		}

	}
	//std::cout << id << std::endl;
	return id;
}
void Model::randomCollapse() {
	int order_for_idx = 0;
	//TODO: design a algorithm to fill the appropriate value into order_for_idx to call the function collapseEdge
	// you are supposed to ensure that each call to this function can reduce the number of face
	
	//实现一个按一定顺序减少面数的算法，确保每次调用函数本身都会减少面数
	//用collapseEdge来真正折叠面
	//选取策略，选取距离最近的边进行折叠
	// 
	//TODO -- END
	std::ofstream fout;
	fout.open("output.txt");
	order_for_idx = findMinIndice();
	/*while (1) {


		int r = 0;
		while (r < halfedges.size()) {
			if (halfedges[r]->valid)
				break;
			r++;
		}

		fout << r << std::endl;
		collapseEdge(r);
	}*/
	collapseEdge(order_for_idx);
	validFaces = statistic_cnt<Face*>(faces);
	validVertices = statistic_cnt<Vertex*>(vertices);
	validEdges = statistic_cnt<Halfedge*>(halfedges);

}
void Model::mergeEdge(int index) {
	//两条边要合并
	if (this->halfedges[index]->next->next->opposite == NULL)
	{
		if (this->halfedges[index]->next->opposite == NULL)
		{
			//消失了
		}
		else {
			this->halfedges[index]->next->opposite->opposite = NULL;
		}
	}
	else {
		if (this->halfedges[index]->next->opposite == NULL)
		{
			//消失了
			this->halfedges[index]->next->next->opposite->opposite = NULL;
		}
		else {
			this->halfedges[index]->next->opposite->opposite = this->halfedges[index]->next->next->opposite;
			this->halfedges[index]->next->next->opposite->opposite = this->halfedges[index]->next->opposite;

		}
	}
	

}
void Model::collapseEdge(int index) {
	//TODO: design a algorithm to collapse the face(edges) with the halfedge[index]

	if (index >= halfedges.size())
		return;
	int start_vertex = this->halfedges[index]->incident_vertex->id;
	int end_vertex = this->halfedges[index]->next->incident_vertex->id;
	//int in_vertex = this->halfedges[index]->next->next->incident_vertex->id;
	if (this->vertices[end_vertex-1]->valid == false || this->vertices[start_vertex-1]->valid == false)
		return;
	if (this->halfedges[index]->opposite != NULL) {
		int oppo_start_vertex = this->halfedges[index]->opposite->incident_vertex->id;

		int oppo_end_vertex = this->halfedges[index]->opposite->next->next->incident_vertex->id;

		//assert(oppo_start_vertex == end_vertex);
	}
	//int oppo_in_vertex = this->halfedges[index]->opposite->next->next->incident_vertex->id;

	this->halfedges[index]->valid = false;
	this->halfedges[index]->next->valid = false;
	this->halfedges[index]->next->next->valid = false;
	//face 消失
	this->halfedges[index]->incident_face->valid = false;
	if (this->halfedges[index]->opposite != NULL) {
		this->halfedges[index]->opposite->valid = false;
		this->halfedges[index]->opposite->next->valid = false;
		this->halfedges[index]->opposite->next->next->valid = false;
		this->halfedges[index]->opposite->incident_face->valid = false;
	}


	//两条边要合并
	mergeEdge(index);
	if (this->halfedges[index]->opposite != NULL)
	{
		mergeEdge(this->halfedges[index]->opposite->id - 1);
	}
	start_vertex = start_vertex - 1;
	end_vertex = end_vertex - 1;
	//两个顶点要合并
	glm::vec3 start = this->vertices[start_vertex]->position;
	glm::vec3 end = this->vertices[end_vertex]->position;


	glm::vec3 temp = glm::vec3((start.x + end.x) / 2, (start.y + end.y) / 2, (start.z + end.z) / 2);
	this->vertices[end_vertex]->position = temp;

	this->vertices[start_vertex]->valid = false;
	this->vertices[end_vertex]->valid = true;
	//修改临边face的起始边为对应的vertices的边
	//有五条边
	//遍历周围的面，把所有vertex id等于起始边的，都给改掉
	//遍历所有的face吧，不想改了
	for (int i = 0; i < this->faces.size(); i++)
	{
		if (faces[i]->valid)
		{

			if (faces[i]->halfedge->incident_vertex->id == start_vertex + 1)
			{
				faces[i]->halfedge->incident_vertex = this->vertices[end_vertex];
			}
			if (faces[i]->halfedge->next->incident_vertex->id == start_vertex + 1)
			{
				faces[i]->halfedge->next->incident_vertex = this->vertices[end_vertex];
			}
			if (faces[i]->halfedge->next->next->incident_vertex->id == start_vertex + 1)
			{
				faces[i]->halfedge->next->next->incident_vertex = this->vertices[end_vertex];
			}

		}
	}

	if (this->vertices[end_vertex]->indcident_halfedge->valid == false && this->halfedges[index]->next->next->opposite != NULL)
	{
		this->vertices[end_vertex]->indcident_halfedge = this->halfedges[index]->next->next->opposite;
	}


	int validCounter = 0;
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, faces.size() * 9 * sizeof(float), nullptr, GL_STATIC_DRAW);

	for (int i = 0; i < faces.size(); i++) {
		if (faces[i]->valid) {
			// only register valid faces;
			Halfedge* current = faces[i]->halfedge;
			if (!current->valid)
			{
				std::cout << "current halfedge whether valid: " << current->valid << std::endl;
			}
			for (int j = 0; j < 3; j++) {
				Vertex* v = current->incident_vertex;
				if (!v->valid)
				{
					std::cout << "current incident_vertex whether valid: " << v->valid << std::endl;
				}

				glBufferSubData(GL_ARRAY_BUFFER, (9 * validCounter + 3 * j) * sizeof(float), 3 * sizeof(float), &(v->position));
				current = current->next;
			}

			++validCounter;
		}
	}
}


void Model::fromMesh(const Mesh& mesh) {
	auto& mvertices = mesh.vertices;
	auto& mindices = mesh.indices;
	std::vector<std::vector<Halfedge*>> map(mvertices.size(), std::vector<Halfedge*>(mvertices.size(), nullptr));
	for (int i = 0; i < mvertices.size(); i++) {
		Vertex* v = new Vertex;
		v->position = mvertices[i]; 
		this->vertices.push_back(v);
	}

	for (int i = 0; i < mindices.size() / 3; i++) {
		int index0 = mindices[3 * i];
		int index1 = mindices[3 * i + 1];
		int index2 = mindices[3 * i + 2];
		Vertex* v0 = this->vertices[index0];
		Vertex* v1 = this->vertices[index1];
		Vertex* v2 = this->vertices[index2];

		Halfedge* edge[3];
		for (int i = 0; i < 3;i++) 
			edge[i] = new Halfedge;
		map[index0][index1] = edge[1];
		map[index1][index2] = edge[2];
		map[index2][index0] = edge[0];
		if (index2 == 1277 || index0 == 1277 || index1 == 1277 )
		{
			int a;
	
		}
		Face* face = new Face;

		// face
		face->halfedge = edge[0];

		// edge
		edge[0]->next = edge[1]; 
		edge[1]->next = edge[2];
		edge[2]->next = edge[0];
		edge[0]->incident_face = face;
		edge[1]->incident_face = face;
		edge[2]->incident_face = face;
		edge[0]->incident_vertex = v0;
		edge[1]->incident_vertex = v1;
		edge[2]->incident_vertex = v2;

		// vertex
		if(v0->indcident_halfedge == nullptr)
			v0->indcident_halfedge = edge[0];
		if(v1->indcident_halfedge == nullptr)
			v1->indcident_halfedge = edge[1];
		if(v2->indcident_halfedge == nullptr)
			v2->indcident_halfedge = edge[2];

		for (int i = 0; i < 3; i++) {
			this->halfedges.push_back(edge[i]);
		}
		this->faces.push_back(face);
	}

	//TODO: you are supposed to fill in the opposite attribute
	//存储方向边，为了实现这个，将起点和终点对调吧
	//半边里存了起始点，下一条半边，下一条半边的起始点就是该半边的终点
	for (int i = 0; i < this->halfedges.size(); i++)
	{
		int start=this->halfedges[i]->incident_vertex->id;
		int end = this->halfedges[i]->next->incident_vertex->id;
		//找到其对应的index
		int index1 = start-1, index2 = end-1;


		//反向边
		Halfedge* half = map[index2][index1];
		if (index2 == 13239)
		{
			int a = 1;
		}
		if (half != NULL)
		{
			this->halfedges[i]->opposite = half->next->next;
			assert(this->halfedges[i]->opposite->incident_vertex->id == this->halfedges[i]->next->incident_vertex->id);
		}
		else
			this->halfedges[i]->opposite = NULL;
		//if (half == NULL)
		//{
		//	int a;
		//	std::cin >> a;
		//}

	}

	//TODO -- END

	validFaces = this->faces.size();
	validEdges = this->halfedges.size();
	validVertices = this->vertices.size();
}

void Model::loadFromFile(const std::string& path) {
	Mesh mesh = Mesh::loadMesh(path);
	fromMesh(mesh);
}

Model::Model() {
	VAO = VBO = EBO = 0;
	dirty = true;
	validFaces = 0;
	validVertices = 0;
	validEdges = 0;
}

Model::~Model() {
	if (VAO) {
		glDeleteVertexArrays(1, &VAO);
		glDeleteBuffers(1, &VBO);
		glDeleteBuffers(1, &EBO);
		
		for (auto p : halfedges) {
			if(p != nullptr)
			delete p;
		}
		for (auto face : faces) {
			if(face !=nullptr)
			delete face;
		}
		for (auto v : vertices) {
			if(v != nullptr)
			delete v;
		}
	}
}

void Model::render(const Shader& shader) {
	if (VAO == 0) {
		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO);
		//glGenBuffers(1, &EBO);

		glBindVertexArray(VAO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);
	}

	int validCounter = 0;
	
	if (dirty) {
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, faces.size() * 9 * sizeof(float), nullptr, GL_STATIC_DRAW);
		for (int i = 0; i < faces.size(); i++) {
			if (faces[i]->valid) {
				// only register valid faces;
				Halfedge* current = faces[i]->halfedge;
				if(!current->valid)
					std::cout << "current halfedge whether valid: " << current->valid << std::endl;
				for (int j = 0; j < 3; j++) {
					Vertex* v = current->incident_vertex;
					if(!v->valid)
						std::cout << "current incident_vertex whether valid: " << v->valid << std::endl;
					glBufferSubData(GL_ARRAY_BUFFER, (9 * validCounter + 3 * j) * sizeof(float), 3 * sizeof(float), &(v->position));
					current = current->next;
				}

				++validCounter;
			}
		}
		dirty = false;
	}

	// rendering
	glBindVertexArray(VAO);
	glDrawArrays(GL_TRIANGLES, 0, validFaces * 3);
}


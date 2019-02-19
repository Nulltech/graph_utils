#include <iostream>
#include <algorithm>
#include <vector>
#include <queue>
#include <ctime>
#include <OpenMesh/Core/IO/MeshIO.hh>

#include <OpenMesh/Core/Mesh/TriMesh_ArrayKernelT.hh>
#include <OpenMesh/Core/Mesh/PolyMesh_ArrayKernelT.hh>

#include <nan.h>

struct CustomTraits : public OpenMesh::DefaultTraits
{
};

typedef OpenMesh::TriMesh_ArrayKernelT<CustomTraits> Mesh;

typedef OpenMesh::PolyMesh_ArrayKernelT<CustomTraits> PolyMesh;

using namespace OpenMesh;
using namespace IO;

Mesh mesh;

double poll_timer(const std::clock_t &start)
{
    return (std::clock() - start) / (double)CLOCKS_PER_SEC;
}

void Method(const Nan::FunctionCallbackInfo<v8::Value> &info)
{
    info.GetReturnValue().Set(Nan::New("world").ToLocalChecked());
}

void loadFile(const Nan::FunctionCallbackInfo<v8::Value> &info)
{
    if (info.Length() < 1)
    {
        Nan::ThrowTypeError("Wrong number of arguments");
        return;
    }
    if (!info[0]->IsString())
    {
        Nan::ThrowTypeError("Wrong arguments type");
        return;
    }
    Nan::Utf8String utf8_value(info[0]);
    std::ifstream t(*utf8_value);
    std::string str((std::istreambuf_iterator<char>(t)),
                    std::istreambuf_iterator<char>());

    std::clock_t start;
    double duration;

    std::string obj = "";

    std::istringstream ss(str);

    OpenMesh::IO::Options opts;
    start = std::clock();
    bool ok = OpenMesh::IO::read_mesh(mesh, ss, "obj", opts);
    std::cout << "Loaded mesh? " << std::boolalpha << ok << std::noboolalpha << std::endl;
    if (!ok)
    {
        return;
    }
    std::cout << std::to_string(poll_timer(start)) << std::endl;
}

void visitMesh(const Nan::FunctionCallbackInfo<v8::Value> &info)
{
    if (info.Length() < 1)
    {
        Nan::ThrowTypeError("Wrong number of arguments");
        return;
    }
    if (!info[0]->IsNumber())
    {
        Nan::ThrowTypeError("Wrong arguments type");
        return;
    }

    int startFace = info[0]->NumberValue();

    std::clock_t start;
    double duration;
    auto faceHandle = mesh.face_handle(startFace);
    auto iterator = mesh.fv_iter(faceHandle);
    auto startVertex = iterator;
    std::deque<OpenMesh::VertexHandle> openHandles;
    std::deque<OpenMesh::VertexHandle> closedHandles;

    openHandles.push_back(*startVertex);

    start = std::clock();
    while (openHandles.size() != 0)
    {
        auto currentHandle = openHandles.front();
        openHandles.pop_front();

        auto present = std::find(closedHandles.begin(), closedHandles.end(), currentHandle);
        if (present != closedHandles.end())
        {
            continue;
        }
        if (mesh.is_boundary(currentHandle))
        {
            // std::cout << "Reached boundary" << std::endl;
            continue;
        }

        for (auto vv_it = mesh.voh_iter(currentHandle); vv_it.is_valid(); ++vv_it)
        {
            auto targetHandle = mesh.to_vertex_handle(vv_it);
            openHandles.push_back(targetHandle);
        }
        closedHandles.push_back(currentHandle);
    }

    std::cout << "Visited mesh of size " << closedHandles.size() << std::endl;
    std::cout << std::to_string(poll_timer(start)) << std::endl;
}

void exportBoundary(const Nan::FunctionCallbackInfo<v8::Value> &info) {
    Mesh clonedMesh;
    OpenMesh::IO::Options opts;
    std::clock_t start;
    double duration;
    start = std::clock();
    // (linearly) iterate over all vertices
    // use an edge iterator to iterate over all the edges
    for (auto eit = mesh.edges_begin(); eit != mesh.edges_end(); ++eit)
    {
        // check for boundary.  (one halfedge won't be valid if boundary)
        // note: you have to dereference the edge iterator
        if (!mesh.is_boundary(*eit))
        {
            // if you want vertex handles use:
            auto vh1 = mesh.to_vertex_handle(mesh.halfedge_handle(*eit, 0));
            auto vh2 = mesh.from_vertex_handle(mesh.halfedge_handle(*eit, 0));

            // if you want handles of faces adjacent to the edge use:
            auto fh1 = mesh.face_handle(mesh.halfedge_handle(*eit, 0));
            auto fh2 = mesh.opposite_face_handle(mesh.halfedge_handle(*eit, 0));
        }
        else
        {
            auto vh1 = mesh.to_vertex_handle(mesh.halfedge_handle(*eit, 0));
            auto vh2 = mesh.from_vertex_handle(mesh.halfedge_handle(*eit, 0));

            clonedMesh.add_vertex(mesh.point(vh1));
            clonedMesh.add_vertex(mesh.point(vh2));
        }
    }
    std::cout << "Iterated along all vertices and cloned boundary" << std::endl;
    std::cout << std::to_string(poll_timer(start)) << std::endl;

    std::ostringstream output_mesh;
    start = std::clock();
    bool writeok = OpenMesh::IO::write_mesh(clonedMesh, "test.obj", opts);
    // bool writeok = OpenMesh::IO::write_mesh(clonedMesh, output_mesh, "obj", opts);
    // std::cout << "Writing mesh: " << std::boolalpha << writeok << std::noboolalpha << std::endl;
    std::cout << std::to_string(poll_timer(start)) << std::endl;
    //   std::cout << output_mesh.str() << std::endl;

    info.GetReturnValue().Set(Nan::New(output_mesh.str().c_str()).ToLocalChecked());
}

void Init(v8::Local<v8::Object> exports)
{
    exports->Set(Nan::New("hello").ToLocalChecked(), Nan::New<v8::FunctionTemplate>(Method)->GetFunction());
    exports->Set(Nan::New("loadFile").ToLocalChecked(), Nan::New<v8::FunctionTemplate>(loadFile)->GetFunction());
    exports->Set(Nan::New("visitMesh").ToLocalChecked(), Nan::New<v8::FunctionTemplate>(visitMesh)->GetFunction());
    exports->Set(Nan::New("exportBoundary").ToLocalChecked(), Nan::New<v8::FunctionTemplate>(exportBoundary)->GetFunction());
}

NODE_MODULE(NODE_GYP_MODULE_NAME, Init)

int main()
{
    std::clock_t start;
    double duration;

    Mesh mesh;
    Mesh clonedMesh;

    std::string obj = "";

    std::istringstream ss(obj);

    OpenMesh::IO::Options opts;
    start = std::clock();
    bool ok = OpenMesh::IO::read_mesh(mesh, ss, "obj", opts);
    std::cout << "Loaded mesh? " << std::boolalpha << ok << std::noboolalpha << std::endl;
    if (!ok)
    {
        return (-1);
    }
    std::cout << std::to_string(poll_timer(start)) << std::endl;

    auto startVertex = mesh.vertices_begin();
    std::deque<OpenMesh::VertexHandle> openHandles;
    std::deque<OpenMesh::VertexHandle> closedHandles;

    openHandles.push_back(*startVertex);

    start = std::clock();
    while (openHandles.size() != 0)
    {
        auto currentHandle = openHandles.front();
        openHandles.pop_front();

        auto present = std::find(closedHandles.begin(), closedHandles.end(), currentHandle);
        if (present != closedHandles.end())
        {
            continue;
        }
        if (mesh.is_boundary(currentHandle))
        {
            // std::cout << "Reached boundary" << std::endl;
            continue;
        }

        for (auto vv_it = mesh.voh_iter(currentHandle); vv_it.is_valid(); ++vv_it)
        {
            auto targetHandle = mesh.to_vertex_handle(vv_it);
            openHandles.push_back(targetHandle);
        }
        closedHandles.push_back(currentHandle);
    }

    std::cout << "Visited mesh of size " << closedHandles.size() << std::endl;
    std::cout << std::to_string(poll_timer(start)) << std::endl;

    start = std::clock();
    // (linearly) iterate over all vertices
    // use an edge iterator to iterate over all the edges
    for (auto eit = mesh.edges_begin(); eit != mesh.edges_end(); ++eit)
    {
        // check for boundary.  (one halfedge won't be valid if boundary)
        // note: you have to dereference the edge iterator
        if (!mesh.is_boundary(*eit))
        {
            // if you want vertex handles use:
            auto vh1 = mesh.to_vertex_handle(mesh.halfedge_handle(*eit, 0));
            auto vh2 = mesh.from_vertex_handle(mesh.halfedge_handle(*eit, 0));

            // if you want handles of faces adjacent to the edge use:
            auto fh1 = mesh.face_handle(mesh.halfedge_handle(*eit, 0));
            auto fh2 = mesh.opposite_face_handle(mesh.halfedge_handle(*eit, 0));
        }
        else
        {
            auto vh1 = mesh.to_vertex_handle(mesh.halfedge_handle(*eit, 0));
            auto vh2 = mesh.from_vertex_handle(mesh.halfedge_handle(*eit, 0));

            clonedMesh.add_vertex(mesh.point(vh1));
            clonedMesh.add_vertex(mesh.point(vh2));
        }
    }
    std::cout << "Iterated along all vertices and cloned boundary" << std::endl;
    std::cout << std::to_string(poll_timer(start)) << std::endl;

    std::ostringstream output_mesh;
    start = std::clock();
    bool writeok = OpenMesh::IO::write_mesh(clonedMesh, output_mesh, "obj", opts);
    std::cout << "Writing mesh: " << std::boolalpha << writeok << std::noboolalpha << std::endl;
    std::cout << std::to_string(poll_timer(start)) << std::endl;
    std::cout << output_mesh.str() << std::endl;
}
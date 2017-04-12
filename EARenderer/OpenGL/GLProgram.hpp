//
//  GLProgram.hpp
//  EARenderer
//
//  Created by Pavlo Muratov on 11.03.17.
//  Copyright © 2017 MPO. All rights reserved.
//

#ifndef GLProgram_hpp
#define GLProgram_hpp

#include <map>
#include <string>

#include "GLNamedObject.hpp"
#include "GLShader.hpp"
#include "GLBindable.hpp"

namespace EARenderer {
    
    class GLProgram: public GLNamedObject, public GLBindable {
    private:
        const GLShader* mVertexShader;
        const GLShader* mFragmentShader;
        std::map<std::string, GLint> mUniforms;
        
        void link();
        void obtainUniforms();
     
    protected:
        GLint uniformLocation(const std::string& name);
        
    public:
        GLProgram(const GLShader* vertexShader, const GLShader* fragmentShader);
        GLProgram(const GLProgram& rhs) = delete;
        GLProgram& operator=(const GLProgram& that) = delete;
        GLProgram(GLProgram&& that) = default;
        GLProgram& operator=(GLProgram&& that) = default;
        virtual ~GLProgram() = 0;
        void swap(GLProgram&);
        
        void bind() const override;
    };
    
    void swap(GLProgram&, GLProgram&);

}

#endif /* GLProgram_hpp */

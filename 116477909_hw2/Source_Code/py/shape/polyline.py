import glm
from OpenGL.GL import *
from .glshape import GLShape
from .renderable import Renderable
from util import Shader

class Polyline(GLShape, Renderable):
    def __init__(self, shader: Shader):
        super().__init__(shader)
        
        # Store the shader and create VAO and VBO for the polyline
        self.shader = shader
        self.vao = glGenVertexArrays(1)
        self.vbo = glGenBuffers(1)
        self.points = glm.array(glm.float32)  # Initialize as empty

    def update_points(self, control_points):
        """Update the polyline points based on control points."""
        # Convert control points to flat array
        self.points = glm.array(glm.float32, *[coord for point in control_points for coord in (point.x, point.y)])

        # Bind VAO and upload data to VBO
        glBindVertexArray(self.vao)
        glBindBuffer(GL_ARRAY_BUFFER, self.vbo)
        glBufferData(GL_ARRAY_BUFFER, self.points.nbytes, self.points.ptr, GL_DYNAMIC_DRAW)
        glEnableVertexAttribArray(0)
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, None)
        glBindBuffer(GL_ARRAY_BUFFER, 0)
        glBindVertexArray(0)

    def render(self, windowWidth, windowHeight):
        """Render the polyline connecting control points and highlight points."""
        if len(self.points) == 0:
            return
        
        # Set shader and window size uniforms
        self.shader.use()
        self.shader.setFloat("windowWidth", windowWidth)
        self.shader.setFloat("windowHeight", windowHeight)

        # Bind VAO for rendering
        glBindVertexArray(self.vao)

        # Render control points as larger highlighted points
        glPointSize(9.0)
        glDrawArrays(GL_POINTS, 0, len(self.points) // 2)  # Divide by 2 because each point has x, y

        # Render the connecting polyline
        glLineWidth(2.0)
        glDrawArrays(GL_LINE_STRIP, 0, len(self.points) // 2)

        # Unbind VAO
        glBindVertexArray(0)

import glm
from OpenGL.GL import *
from .glshape import GLShape
from .renderable import Renderable
from util import Shader

class BezierCurve(GLShape, Renderable):
    def __init__(self, shader: Shader, control_points: glm.array, num_segments=100):
        super().__init__(shader)
        assert len(control_points) == 8, "BezierCurve requires exactly four control points (as x, y pairs)."
        
        # Store shader and control points
        self.shader = shader
        self.control_points = control_points
        self.num_segments = num_segments  # Number of points along the curve

        # Generate points along the Bezier curve
        self.curve_points = self.calculate_bezier_points()

        # VAO and VBO for the Bezier curve
        self.vao = glGenVertexArrays(1)
        self.vbo = glGenBuffers(1)

        # Bind VAO and upload curve points data to VBO
        glBindVertexArray(self.vao)
        glBindBuffer(GL_ARRAY_BUFFER, self.vbo)
        glBufferData(GL_ARRAY_BUFFER, self.curve_points.nbytes, self.curve_points.ptr, GL_STATIC_DRAW)

        # Set vertex attribute pointers
        glEnableVertexAttribArray(0)
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, None)

        # Unbind VAO
        glBindBuffer(GL_ARRAY_BUFFER, 0)
        glBindVertexArray(0)

    def calculate_bezier_points(self):
        """Calculate points along the cubic Bezier curve."""
        points = []
        p0 = glm.vec2(self.control_points[0], self.control_points[1])
        p1 = glm.vec2(self.control_points[2], self.control_points[3])
        p2 = glm.vec2(self.control_points[4], self.control_points[5])
        p3 = glm.vec2(self.control_points[6], self.control_points[7])
        
        # Calculate points along the Bezier curve
        for i in range(self.num_segments + 1):
            t = i / self.num_segments
            point = (1 - t)**3 * p0 + 3 * (1 - t)**2 * t * p1 + 3 * (1 - t) * t**2 * p2 + t**3 * p3
            points.extend([point.x, point.y])
        
        return glm.array(glm.float32, *points)

    def render(self, timeElapsedSinceLastFrame: int, animate: bool) -> None:
        self.shader.use()
        glBindVertexArray(self.vao)
        glDrawArrays(GL_LINE_STRIP, 0, self.num_segments + 1)
        glBindVertexArray(0)

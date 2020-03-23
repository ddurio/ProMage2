<Shader>
    <Pass source="../Code/Submodule/Engine/Code/Engine/Renderer/Shaders/DefaultUnlit.hlsl" defines="">
        <Vertex entry="VertexFunction"/>
        <Fragment entry="FragmentFunction"/>
        <Depth write="false" compareOp="always"/>
        <Blend>
            <Color src="srcAlpha" dst="invSrcAlpha" op="add"/>
            <Alpha src="one" dst="one" op="max"/>
        </Blend>
    </Pass>
</Shader>
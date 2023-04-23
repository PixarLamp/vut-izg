/*!
 * @file
 * @brief This file contains implementation of gpu
 *
 * @author Tomáš Milet, imilet@fit.vutbr.cz
 */

#include <student/gpu.hpp>
#include <stdio.h>
#include <stdexcept>
#include <stdlib.h>

/// \addtogroup gpu_init
/// @{

/**
 * @brief Constructor of GPU
 */
GPU::GPU(){
    buffer = 0;
    address = NULL;
    vao = 0;
    position = 0;
    activePuller = 0;
    prg = 0;
    activeprog = 0;
}

/**
 * @brief Destructor of GPU
 */
GPU::~GPU(){
    while (vao != 0) {
        deleteBuffer(vao);
        vao -= 1;
    }
  /// \todo Zde můžete dealokovat/deinicializovat grafickou kartu
}

/// @}

/** \addtogroup buffer_tasks 01. Implementace obslužných funkcí pro buffery
 * @{
 */

/**
 * @brief This function allocates buffer on GPU.
 *
 * @param size size in bytes of new buffer on GPU.
 *
 * @return unique identificator of the buffer
 */
BufferID GPU::createBuffer(uint64_t size) {
    address = new uint8_t[size];
    if (address != NULL){
        buffer += 1;
        buffers.insert(std::pair<BufferID, uint8_t*>(buffer, address));
        return buffer;
    }
    else
        return emptyID;
  /// \todo Tato funkce by měla na grafické kartě vytvořit buffer dat.<br>
  /// Velikost bufferu je v parameteru size (v bajtech).<br>
  /// Funkce by měla vrátit unikátní identifikátor identifikátor bufferu.<br>
  /// Na grafické kartě by mělo být možné alkovat libovolné množství bufferů o libovolné velikosti.<br>
}

/**
 * @brief This function frees allocated buffer on GPU.
 *
 * @param buffer buffer identificator
 */
void GPU::deleteBuffer(BufferID buffer) {
    if (isBuffer(buffer) == true) {
        delete(buffers.at(buffer));
        buffers.erase(buffer);
    }
  /// \todo Tato funkce uvolní buffer na grafické kartě.
  /// Buffer pro smazání je vybrán identifikátorem v parameteru "buffer".
  /// Po uvolnění bufferu je identifikátor volný a může být znovu použit při vytvoření nového bufferu.
}

/**
 * @brief This function uploads data to selected buffer on the GPU
 *
 * @param buffer buffer identificator
 * @param offset specifies the offset into the buffer's data
 * @param size specifies the size of buffer that will be uploaded
 * @param data specifies a pointer to new data
 */
void GPU::setBufferData(BufferID buffer, uint64_t offset, uint64_t size, void const* data) {
    if (isBuffer(buffer) == true) {
        address = buffers.at(buffer);
        std::memcpy(address + offset, data, size);
    }
  /// \todo Tato funkce nakopíruje data z cpu na "gpu".<br>
  /// Data by měla být nakopírována do bufferu vybraného parametrem "buffer".<br>
  /// Parametr size určuje, kolik dat (v bajtech) se překopíruje.<br>
  /// Parametr offset určuje místo v bufferu (posun v bajtech) kam se data nakopírují.<br>
  /// Parametr data obsahuje ukazatel na data na cpu pro kopírování.<br>
}

/**
 * @brief This function downloads data from GPU.
 *
 * @param buffer specfies buffer
 * @param offset specifies the offset into the buffer from which data will be returned, measured in bytes. 
 * @param size specifies data size that will be copied
 * @param data specifies a pointer to the location where buffer data is returned. 
 */
void GPU::getBufferData(BufferID buffer,
                        uint64_t offset,
                        uint64_t size,
                        void*    data)
{
    if (isBuffer(buffer) == true) {
        address = buffers.at(buffer) + offset;
        std::memcpy(data, address, size);
    }
  /// \todo Tato funkce vykopíruje data z "gpu" na cpu.
  /// Data by měla být vykopírována z bufferu vybraného parametrem "buffer".<br>
  /// Parametr size určuje kolik dat (v bajtech) se překopíruje.<br>
  /// Parametr offset určuje místo v bufferu (posun v bajtech) odkud se začne kopírovat.<br>
  /// Parametr data obsahuje ukazatel, kam se data nakopírují.<br>
}

/**
 * @brief This function tests if buffer exists
 *
 * @param buffer selected buffer id
 *
 * @return true if buffer points to existing buffer on the GPU.
 */
bool GPU::isBuffer(BufferID buffer) {
    try {
        buffers.at(buffer);
    }
    catch (std::out_of_range) {
        return false;   
    }
    return true;
  /// \todo Tato funkce by měla vrátit true pokud buffer je identifikátor existující bufferu.<br>
  /// Tato funkce by měla vrátit false, pokud buffer není identifikátor existujícího bufferu. (nebo bufferu, který byl smazán).<br>
  /// Pro emptyId vrací false.<br>
}

/// @}

/**
 * \addtogroup vertexpuller_tasks 02. Implementace obslužných funkcí pro vertex puller
 * @{
 */

/**
 * @brief This function creates new vertex puller settings on the GPU,
 *
 * @return unique vertex puller identificator
 */
ObjectID GPU::createVertexPuller     (){
    vao += 1;
    verpul table;
    vertextable.insert(std::pair<VertexPullerID, verpul>(vao, table));
    try {
        vertextable.at(vao);
    }
    catch (std::out_of_range&) {
        return emptyID;
    }
    return vao;

  /// \todo Tato funkce vytvoří novou práznou tabulku s nastavením pro vertex puller.<br>
  /// Funkce by měla vrátit identifikátor nové tabulky.
  /// Prázdná tabulka s nastavením neobsahuje indexování a všechny čtecí hlavy jsou vypnuté.
}

/**
 * @brief This function deletes vertex puller settings
 *
 * @param vao vertex puller identificator
 */
void     GPU::deleteVertexPuller     (VertexPullerID vao){
    if (isVertexPuller(vao) == true) {
        vertextable.erase(vao);
    }
  /// \todo Tato funkce by měla odstranit tabulku s nastavení pro vertex puller.<br>
  /// Parameter "vao" obsahuje identifikátor tabulky s nastavením.<br>
  /// Po uvolnění nastavení je identifiktátor volný a může být znovu použit.<br>
}

/**
 * @brief This function sets one vertex puller reading head.
 *
 * @param vao identificator of vertex puller
 * @param head id of vertex puller head
 * @param type type of attribute
 * @param stride stride in bytes
 * @param offset offset in bytes
 * @param buffer id of buffer
 */
void     GPU::setVertexPullerHead    (VertexPullerID vao,uint32_t head,AttributeType type,uint64_t stride,uint64_t offset,BufferID buffer){
    if (isVertexPuller(vao) == true) {
        verpul& tab = vertextable.at(vao);
        tab.head[head].buffer = buffer;
        tab.head[head].offset = offset;
        tab.head[head].stride = stride;
        tab.head[head].type = type;
    }
  /// \todo Tato funkce nastaví jednu čtecí hlavu vertex pulleru.<br>
  /// Parametr "vao" vybírá tabulku s nastavením.<br>
  /// Parametr "head" vybírá čtecí hlavu vybraného vertex pulleru.<br>
  /// Parametr "type" nastaví typ atributu, který čtecí hlava čte. Tímto se vybere kolik dat v bajtech se přečte.<br>
  /// Parametr "stride" nastaví krok čtecí hlavy.<br>
  /// Parametr "offset" nastaví počáteční pozici čtecí hlavy.<br>
  /// Parametr "buffer" vybere buffer, ze kterého bude čtecí hlava číst.<br>
}

/**
 * @brief This function sets vertex puller indexing.
 *
 * @param vao vertex puller id
 * @param type type of index
 * @param buffer buffer with indices
 */
void     GPU::setVertexPullerIndexing(VertexPullerID vao,IndexType type,BufferID buffer){
    if (isVertexPuller(vao) == true) {
        verpul& tab = vertextable.at(vao);
        tab.index.buffer = buffer;
        tab.index.type = type;
    }
  /// \todo Tato funkce nastaví indexování vertex pulleru.
  /// Parametr "vao" vybírá tabulku s nastavením.<br>
  /// Parametr "type" volí typ indexu, který je uložený v bufferu.<br>
  /// Parametr "buffer" volí buffer, ve kterém jsou uloženy indexy.<br>
}

/**
 * @brief This function enables vertex puller's head.
 *
 * @param vao vertex puller 
 * @param head head id
 */
void     GPU::enableVertexPullerHead (VertexPullerID vao,uint32_t head){
    if (isVertexPuller(vao) == true) {
        verpul& tab = vertextable.at(vao);
        tab.head[head].state = true;
    }
  /// \todo Tato funkce povolí čtecí hlavu daného vertex pulleru.<br>
  /// Pokud je čtecí hlava povolena, hodnoty z bufferu se budou kopírovat do atributu vrcholů vertex shaderu.<br>
  /// Parametr "vao" volí tabulku s nastavením vertex pulleru (vybírá vertex puller).<br>
  /// Parametr "head" volí čtecí hlavu.<br>
}

/**
 * @brief This function disables vertex puller's head
 *
 * @param vao vertex puller id
 * @param head head id
 */
void     GPU::disableVertexPullerHead(VertexPullerID vao,uint32_t head){
    if (isVertexPuller(vao) == true) {
        verpul& tab = vertextable.at(vao);
        tab.head[head].state = false;
    }
  /// \todo Tato funkce zakáže čtecí hlavu daného vertex pulleru.<br>
  /// Pokud je čtecí hlava zakázána, hodnoty z bufferu se nebudou kopírovat do atributu vrcholu.<br>
  /// Parametry "vao" a "head" vybírají vertex puller a čtecí hlavu.<br>
}

/**
 * @brief This function selects active vertex puller.
 *
 * @param vao id of vertex puller
 */
void     GPU::bindVertexPuller       (VertexPullerID vao){
    if (isVertexPuller(vao) == true) {
        activePuller = vao;
    }
  /// \todo Tato funkce aktivuje nastavení vertex pulleru.<br>
  /// Pokud je daný vertex puller aktivován, atributy z bufferů jsou vybírány na základě jeho nastavení.<br>
}

/**
 * @brief This function deactivates vertex puller.
 */
void     GPU::unbindVertexPuller     (){
    activePuller = emptyID;
  /// \todo Tato funkce deaktivuje vertex puller.
  /// To většinou znamená, že se vybere neexistující "emptyID" vertex puller.
}

/**
 * @brief This function tests if vertex puller exists.
 *
 * @param vao vertex puller
 *
 * @return true, if vertex puller "vao" exists
 */
bool     GPU::isVertexPuller         (VertexPullerID vao){
    try {
        vertextable.at(vao);
    }
    catch (std::out_of_range&) {
        return false;
    }
    return true;
  /// \todo Tato funkce otestuje, zda daný vertex puller existuje.
  /// Pokud ano, funkce vrací true.
}

/// @}

/** \addtogroup program_tasks 03. Implementace obslužných funkcí pro shader programy
 * @{
 */

/**
 * @brief This function creates new shader program.
 *
 * @return shader program id
 */
ProgramID        GPU::createProgram         (){
    prg += 1;
    programset prog;
    prog.type = AttributeType::EMPTY;
    programs.insert(std::make_pair(prg, prog));
    try {
        programs.at(prg);
    }
    catch (std::out_of_range&) {
        return emptyID;
    }
    return prg;
  /// \todo Tato funkce by měla vytvořit nový shader program.<br>
  /// Funkce vrací unikátní identifikátor nového proramu.<br>
  /// Program je seznam nastavení, které obsahuje: ukazatel na vertex a fragment shader.<br>
  /// Dále obsahuje uniformní proměnné a typ výstupních vertex attributů z vertex shaderu, které jsou použity pro interpolaci do fragment atributů.<br>
  return emptyID;
}

/**
 * @brief This function deletes shader program
 *
 * @param prg shader program id
 */
void             GPU::deleteProgram         (ProgramID prg){
    if (isProgram(prg) == true) {
        programs.erase(prg);
    }
  /// \todo Tato funkce by měla smazat vybraný shader program.<br>
  /// Funkce smaže nastavení shader programu.<br>
  /// Identifikátor programu se stane volným a může být znovu využit.<br>
}

/**
 * @brief This function attaches vertex and frament shader to shader program.
 *
 * @param prg shader program
 * @param vs vertex shader 
 * @param fs fragment shader
 */
void             GPU::attachShaders         (ProgramID prg,VertexShader vs,FragmentShader fs){
    if (isProgram(prg) == true) {
        programset& neededprog = programs.at(prg);
        neededprog.fs = fs;
        neededprog.vs = vs;
    }
    /// \todo Tato funkce by měla připojít k vybranému shader programu vertex a fragment shader.
}

/**
 * @brief This function selects which vertex attributes should be interpolated during rasterization into fragment attributes.
 *
 * @param prg shader program
 * @param attrib id of attribute
 * @param type type of attribute
 */
void             GPU::setVS2FSType          (ProgramID prg,uint32_t attrib,AttributeType type){
    if (isProgram(prg) == true) {
        programset& neededprog = programs.at(prg);
        neededprog.attrib = attrib;
        neededprog.type = type;
    }
  /// \todo tato funkce by měla zvolit typ vertex atributu, který je posílán z vertex shaderu do fragment shaderu.<br>
  /// V průběhu rasterizace vznikají fragment.<br>
  /// Fragment obsahují fragment atributy.<br>
  /// Tyto atributy obsahují interpolované hodnoty vertex atributů.<br>
  /// Tato funkce vybere jakého typu jsou tyto interpolované atributy.<br>
  /// Bez jakéhokoliv nastavení jsou atributy prázdne AttributeType::EMPTY<br>
}

/**
 * @brief This function actives selected shader program
 *
 * @param prg shader program id
 */
void             GPU::useProgram            (ProgramID prg){
    if (isProgram(prg) == true) {
        activeprog = prg;
    }
  /// \todo tato funkce by měla vybrat aktivní shader program.
}

/**
 * @brief This function tests if selected shader program exists.
 *
 * @param prg shader program
 *
 * @return true, if shader program "prg" exists.
 */
bool             GPU::isProgram             (ProgramID prg){
    try {
        programs.at(prg);
    }
    catch (std::out_of_range&) {
        return false;
    }
    return true;
  /// \todo tato funkce by měla zjistit, zda daný program existuje.<br>
  /// Funkce vráti true, pokud program existuje.<br>
  return false;
}

/**
 * @brief This function sets uniform value (1 float).
 *
 * @param prg shader program
 * @param uniformId id of uniform value (number of uniform values is stored in maxUniforms variable)
 * @param d value of uniform variable
 */
void             GPU::programUniform1f      (ProgramID prg,uint32_t uniformId,float     const&d){
    if (isProgram(prg) == true) {
        programset& neededprog = programs.at(prg);
        neededprog.uni.uniform[uniformId].v1 = d;
    }
  /// \todo tato funkce by měla nastavit uniformní proměnnou shader programu.<br>
  /// Parametr "prg" vybírá shader program.<br>
  /// Parametr "uniformId" vybírá uniformní proměnnou. Maximální počet uniformních proměnných je uložen v programné \link maxUniforms \endlink.<br>
  /// Parametr "d" obsahuje data (1 float).<br>
}

/**
 * @brief This function sets uniform value (2 float).
 *
 * @param prg shader program
 * @param uniformId id of uniform value (number of uniform values is stored in maxUniforms variable)
 * @param d value of uniform variable
 */
void             GPU::programUniform2f      (ProgramID prg,uint32_t uniformId,glm::vec2 const&d){
    if (isProgram(prg) == true) {
        programset& neededprog = programs.at(prg);
        neededprog.uni.uniform[uniformId].v2 = d;
    }
  /// \todo tato funkce dělá obdobnou věc jako funkce programUniform1f.<br>
  /// Místo 1 floatu nahrává 2 floaty.
}

/**
 * @brief This function sets uniform value (3 float).
 *
 * @param prg shader program
 * @param uniformId id of uniform value (number of uniform values is stored in maxUniforms variable)
 * @param d value of uniform variable
 */
void             GPU::programUniform3f      (ProgramID prg,uint32_t uniformId,glm::vec3 const&d){
    if (isProgram(prg) == true) {
        programset& neededprog = programs.at(prg);
        neededprog.uni.uniform[uniformId].v3 = d;
    }
  /// \todo tato funkce dělá obdobnou věc jako funkce programUniform1f.<br>
  /// Místo 1 floatu nahrává 3 floaty.
}

/**
 * @brief This function sets uniform value (4 float).
 *
 * @param prg shader program
 * @param uniformId id of uniform value (number of uniform values is stored in maxUniforms variable)
 * @param d value of uniform variable
 */
void             GPU::programUniform4f      (ProgramID prg,uint32_t uniformId,glm::vec4 const&d){
    if (isProgram(prg) == true) {
        programset& neededprog = programs.at(prg);
        neededprog.uni.uniform[uniformId].v4 = d;
    }
  /// \todo tato funkce dělá obdobnou věc jako funkce programUniform1f.<br>
  /// Místo 1 floatu nahrává 4 floaty.
}

/**
 * @brief This function sets uniform value (4 float).
 *
 * @param prg shader program
 * @param uniformId id of uniform value (number of uniform values is stored in maxUniforms variable)
 * @param d value of uniform variable
 */
void             GPU::programUniformMatrix4f(ProgramID prg,uint32_t uniformId,glm::mat4 const&d){
    if (isProgram(prg) == true) {
        programset& neededprog = programs.at(prg);
        neededprog.uni.uniform[uniformId].m4 = d;
    }
  /// \todo tato funkce dělá obdobnou věc jako funkce programUniform1f.<br>
  /// Místo 1 floatu nahrává matici 4x4 (16 floatů).
}

/// @}





/** \addtogroup framebuffer_tasks 04. Implementace obslužných funkcí pro framebuffer
 * @{
 */

/**
 * @brief This function creates framebuffer on GPU.
 *
 * @param width width of framebuffer
 * @param height height of framebuffer
 */
void GPU::createFramebuffer      (uint32_t width,uint32_t height){
    framebuffer.width = width;
    framebuffer.height = height;
    framebuffer.colour = new uint8_t[width * height * 4];
    framebuffer.depth = new float[width * height];
  /// \todo Tato funkce by měla alokovat framebuffer od daném rozlišení.<br>
  /// Framebuffer se skládá z barevného a hloukového bufferu.<br>
  /// Buffery obsahují width x height pixelů.<br>
  /// Barevný pixel je složen z 4 x uint8_t hodnot - to reprezentuje RGBA barvu.<br>
  /// Hloubkový pixel obsahuje 1 x float - to reprezentuje hloubku.<br>
  /// Nultý pixel framebufferu je vlevo dole.<br>
}

/**
 * @brief This function deletes framebuffer.
 */
void GPU::deleteFramebuffer      (){
    delete(framebuffer.colour);
    delete(framebuffer.depth);
  /// \todo tato funkce by měla dealokovat framebuffer.
}

/**
 * @brief This function resizes framebuffer.
 *
 * @param width new width of framebuffer
 * @param height new heght of framebuffer
 */
void     GPU::resizeFramebuffer(uint32_t width,uint32_t height){
    deleteFramebuffer();
    createFramebuffer(width, height);
  /// \todo Tato funkce by měla změnit velikost framebuffer.
}

/**
 * @brief This function returns pointer to color buffer.
 *
 * @return pointer to color buffer
 */
uint8_t* GPU::getFramebufferColor  (){
    if (framebuffer.colour != NULL) {
        return framebuffer.colour;
    }
  /// \todo Tato funkce by měla vrátit ukazatel na začátek barevného bufferu.<br>
  return nullptr;
}

/**
 * @brief This function returns pointer to depth buffer.
 *
 * @return pointer to dept buffer.
 */
float* GPU::getFramebufferDepth    (){
    if (framebuffer.depth != NULL) {
        return framebuffer.depth;
    }
  /// \todo tato funkce by mla vrátit ukazatel na začátek hloubkového bufferu.<br>
  return nullptr;
}

/**
 * @brief This function returns width of framebuffer
 *
 * @return width of framebuffer
 */
uint32_t GPU::getFramebufferWidth (){
    if (framebuffer.width != NULL) {
        return framebuffer.width;
    }
  /// \todo Tato funkce by měla vrátit šířku framebufferu.
  return 0;
}

/**
 * @brief This function returns height of framebuffer.
 *
 * @return height of framebuffer
 */
uint32_t GPU::getFramebufferHeight(){
    if (framebuffer.height != NULL) {
        return framebuffer.height;
    }
  /// \todo Tato funkce by měla vrátit výšku framebufferu.
  return 0;
}

/// @}

/** \addtogroup draw_tasks 05. Implementace vykreslovacích funkcí
 * Bližší informace jsou uvedeny na hlavní stránce dokumentace.
 * @{
 */

/**
 * @brief This functino clears framebuffer.
 *
 * @param r red channel
 * @param g green channel
 * @param b blue channel
 * @param a alpha channel
 */
void            GPU::clear                 (float r,float g,float b,float a){
    for (uint32_t index = 0; index < framebuffer.height * framebuffer.width * 4; index += 4) {
        framebuffer.colour[index] = uint8_t(255 * r);
        framebuffer.colour[index + 1] = uint8_t(255 * g);
        framebuffer.colour[index + 2] = uint8_t(255 * b);
        framebuffer.colour[index + 3] = uint8_t(255 * a);
    }
  /// \todo Tato funkce by měla vyčistit framebuffer.<br>
  /// Barevný buffer vyčistí na barvu podle parametrů r g b a (0 - nulová intenzita, 1 a větší - maximální intenzita).<br>
  /// (0,0,0) - černá barva, (1,1,1) - bílá barva.<br>
  /// Hloubkový buffer nastaví na takovou hodnotu, která umožní rasterizaci trojúhelníka, který leží v rámci pohledového tělesa.<br>
  /// Hloubka by měla být tedy větší než maximální hloubka v NDC (normalized device coordinates).<br>
}

InVertex        GPU::PullVertex            (int ind) {
    InVertex pulledvertexes;
    verpul& tab = vertextable.at(activePuller);
    pulledvertexes.gl_VertexID = ind;

    if (tab.index.buffer != emptyID) {
        memcpy(&pulledvertexes.gl_VertexID, &buffers.at(tab.index.buffer)[0] + (int)tab.index.type * ind, (int)tab.index.type);
    }

    for (int i = 0; i < maxAttributes; i += 1){
        if (tab.head[i].state == true) {
            getBufferData(tab.head[i].buffer, tab.head[i].offset + tab.head[i].stride * pulledvertexes.gl_VertexID, 4*(int)tab.head[i].type, &pulledvertexes.attributes[i]);
        }
    }
 
    return pulledvertexes;
}

void            GPU::drawTriangles         (uint32_t  nofVertices){

    for (int j = 0; j < nofVertices; j += 1) {
        InVertex ivertex = PullVertex(j);
        programset& neededprog = programs.at(activeprog);
        OutVertex overtex;
        neededprog.vs(overtex, ivertex, neededprog.uni);
    }
  /// \todo Tato funkce vykreslí trojúhelníky podle daného nastavení.<br> 
  /// Vrcholy se budou vybírat podle nastavení z aktivního vertex pulleru (pomocí bindVertexPuller).<br>
  /// Vertex shader a fragment shader se zvolí podle aktivního shader programu (pomocí useProgram).<br>
  /// Parametr "nofVertices" obsahuje počet vrcholů, který by se měl vykreslit (3 pro jeden trojúhelník).<br>
}

/// @}

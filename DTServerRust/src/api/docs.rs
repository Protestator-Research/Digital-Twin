//! The documentation of the API, served by the API itself: the OpenAPI document and a Swagger UI page for it. All
//! files are embedded in the binary (`docs/api/`, the UI is `swagger-ui-dist`, see `docs/api/swagger-ui/VERSION`), so
//! the page works offline and makes no request to any other host.
//!
//! The routes are part of [`super::router`] and therefore behind the same authentication as the data routes.
//!
//! - `/docs/` is the page. Its references are relative (`swagger-ui/...`, `openapi.yaml`), so they resolve below
//!   `/docs/`; `/docs` redirects there (temporarily and relative, so it also works behind a proxy that adds a path
//!   prefix), as the references would resolve against `/` otherwise.
//! - `/openapi.yaml` and `/docs/openapi.yaml` are the same document.

use axum::http::{header, HeaderValue};
use axum::response::{IntoResponse, Redirect, Response};
use axum::routing::get;
use axum::Router;

use super::ApiState;

const INDEX: &str = include_str!("../../docs/api/index.html");
const OPENAPI: &str = include_str!("../../docs/api/openapi.yaml");
const SWAGGER_CSS: &str = include_str!("../../docs/api/swagger-ui/swagger-ui.css");
const SWAGGER_JS: &str = include_str!("../../docs/api/swagger-ui/swagger-ui-bundle.js");

/// The routes of the documentation.
pub(super) fn routes() -> Router<ApiState> {
    Router::new()
        .route("/docs", get(|| async { Redirect::temporary("docs/") }))
        .route(
            "/docs/",
            get(|| async { file(INDEX, "text/html; charset=utf-8") }),
        )
        .route(
            "/docs/swagger-ui/swagger-ui.css",
            get(|| async { file(SWAGGER_CSS, "text/css; charset=utf-8") }),
        )
        .route(
            "/docs/swagger-ui/swagger-ui-bundle.js",
            get(|| async { file(SWAGGER_JS, "text/javascript; charset=utf-8") }),
        )
        .route(
            "/docs/openapi.yaml",
            get(|| async { file(OPENAPI, "application/yaml") }),
        )
        .route(
            "/openapi.yaml",
            get(|| async { file(OPENAPI, "application/yaml") }),
        )
}

fn file(body: &'static str, content_type: &'static str) -> Response {
    let mut response = body.into_response();
    response
        .headers_mut()
        .insert(header::CONTENT_TYPE, HeaderValue::from_static(content_type));
    response
}

/// The embedded OpenAPI document (also used by the tests).
pub const OPENAPI_YAML: &str = OPENAPI;
